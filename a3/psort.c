#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "helper.h"

// the main code block

int main(int argc, char* argv[]){
    //check input
    if (argc != 7){
        check_usage();
    }
    int opt = 0;
    char * filename;
    char * output;
    int num_processes;
    while ((opt = getopt(argc, argv, ":n:f:o:")) != -1){
        switch(opt){
            case 'n':
                num_processes = strtol(optarg, NULL, 10);
                if (num_processes < 1){
                    printf("Invalid argument: number of processes must be greater than 0\n");
                    exit(0);
                }
                break;
            case 'f':
                filename = optarg;
                break;
            case 'o':
                output = optarg;
                break;
            default:
                check_usage();
        }
    }
    
    // some variables
    
    int filesize = get_file_size(filename);
    int num_entries = filesize / sizeof(struct rec);
    if (num_entries < 1){
        fprintf(stderr, "There are no records to read from file.\n");
        exit(0);
    }
    
    // generate pipes
    int min_entity = 0;
    if (num_processes <= num_entries){
        min_entity = num_processes;
    } else {
        min_entity = num_entries;
    }
    int fd[min_entity-1][2];
    
    
    // have an array for deciding the work
    
    int * work = malloc(sizeof(int) * min_entity);
    delegate_work(work, min_entity, num_entries);
    
    // start calling Fork
    
    for (int a = 0; a < min_entity; a++){
        Pipe(fd[a]);
        int forkval = Fork();
        if (forkval == 0){
            // in child, open the file
            int num_records_inside_process = work[a];
            struct rec child_records[num_records_inside_process];
            FILE * fp = Fopen(filename, "rb");
            int bytes_to_skip = get_bytes_to_skip(a, work);
            //read from the file and store that stuff in an array
            fseek(fp, bytes_to_skip, SEEK_SET);
            for (int b = 0; b < num_records_inside_process; b++){
                fread(&(child_records[b]), sizeof(struct rec), 1, fp);
            }
            //close the reading end of the pipe
            Close(fd[a][0]);
            // call qsort
            qsort(child_records, num_records_inside_process, sizeof(struct rec), compare_freq);
            // write to a pipe
            for (int c = 0; c < num_records_inside_process; c++){
                Write(fd[a][1], &(child_records[c]), sizeof(struct rec));
            }
            Close(fd[a][1]);
            Fclose(fp);
            free(work);
            exit(0);
        } else {
            // close the writing end of the file
            Close(fd[a][1]);
        }
    }
    
    // 1. first for loop, till num_processes, that reads from pipes of parent into temporary array
    struct rec records_finished;
    records_finished.freq = -1;
    strncpy(records_finished.word,"finished",44);
    struct rec buffer[min_entity];
    for (int d = 0; d < min_entity; d++){
        Read(fd[d][0], &(buffer[d]), sizeof(struct rec), &records_finished);
    }
    
    // 2. in a for loop going to the num_entries, get minimum from temp array, and dump it into a file
    struct rec minimum_rec;
    int minimum_index;
    FILE * fp_out = Fopen(output, "wb");
    for (int e = 0; e < num_entries; e++){
        get_minimum_struct(&minimum_rec, &minimum_index, buffer, min_entity);
        Fwrite(&minimum_rec, sizeof(struct rec), 1, fp_out);
        if (read(fd[minimum_index][0], &(buffer[minimum_index]), sizeof(struct rec)) == 0){
            Close(fd[minimum_index][0]);
            buffer[minimum_index] = records_finished;
        }
    }
    
    // 3. close up anything thats left, and free up all the malloc'd memory
    Fclose(fp_out);
    for (int t = 0; t < min_entity; t++){
        // in the parent, call wait, store the exit status into an int *
        int exit_sig;
        Wait(&exit_sig);
        if (WIFEXITED(exit_sig)){
            WEXITSTATUS(exit_sig);
            if (exit_sig != 0){
                fprintf(stderr, "Child terminated abnormally\n");
            }
        }
    }
    free(work);
    return 0;
}
