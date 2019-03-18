#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "helper.h"

// checker and wrapper functions

void check_usage(){
  fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
  exit(1);
}

FILE * Fopen(char * filename, char * mode){
  FILE * fp = fopen(filename, mode);
  if (fp == NULL){
    fprintf(stderr, "fopen failure\n");
    exit(1);
  }
  return fp;
}

int Close(int file_des){
  int result = close(file_des);
  if (result == -1){
    perror("close");
    exit(1);
  }
  return result;
}

int Fork(){
  int result = fork();
  if (result < 0){
    perror("fork");
    exit(1);
  }
  return result;
}

int Wait(int * exit_sig){
  int result = wait(exit_sig);
  if (result == -1){
    perror("wait");
    exit(1);
  }
  return result;
}

int Fwrite(void * ptr, size_t size, size_t nitems,FILE * stream){
  int test = fwrite(ptr, size, nitems, stream);
  if (test < 0){
    perror("fwrite");
    exit(1);
  } else if (test == 0){
    printf("nothing was written by fwrite\n");
  }
  return test;
}

void Pipe(int * fd){
  if ((pipe(fd))==-1){
    perror("pipe");
    exit(1);
  }
}

void * Malloc(size_t size){
  void * ptr = malloc(size);
  if (ptr == NULL){
    perror("malloc");
    exit(1);
  }
  return ptr;
}

int Write(int fd, void * pointer, size_t size){
  int t = write(fd, pointer, size);
  if (t < 0){
    perror("write");
    exit(1);
  }
  return t;
}

int Read(int file_des, void *buffer, size_t count, struct rec * ifempty){
  int result = read(file_des, buffer, count);
  if (result < 0){
    perror("read");
    exit(1);
  }
  return result;
 }

int Fclose(FILE * fd){
  int check = fclose(fd);
  if (check != 0){
    fprintf(stderr, "fclose failure\n");
    exit(1);
  }
  return check;
}

// Helper functions for the program

void delegate_work(int work[], int num_processes, int num_entries){
  int remainder = num_entries % num_processes;
  int multiple = num_entries - remainder;
  for (int i = 0; i < num_processes; i++){
    work[i] = multiple / num_processes;
  }
  int j = 0;
  while (remainder > 0){
    work[j] ++;
    j++;
    remainder = remainder - 1;
  }
}

int get_bytes_to_skip(int iteration, int * work){
  int sum = 0;
  for (int i = 0; i < iteration; i++){
    sum += work[i];
  }
  return (sum * sizeof(struct rec));
}

void get_minimum_struct(struct rec * rec_ptr, int * index_ptr, struct rec * buffer, int num_processes){
  int minimum_index = 0;
  struct rec minimum_struct;
  minimum_struct.freq =  2147483647;
  for (int i = 0; i < num_processes; i++){
    if ((buffer[i].freq <= minimum_struct.freq) && (buffer[i].freq != -1)){
      minimum_index = i;
      minimum_struct = buffer[i];
    }
  }
  *index_ptr = minimum_index;
  *rec_ptr = minimum_struct;
}

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
          exit(1);
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
      // see if freeing work is required in here
      free(work);
      exit(0);
    } else {
      // in the parent, call wait, store the exit status into an int *
      Wait(NULL);

      // close the writing end of the file
      Close(fd[a][1]);
    }
  }
  // outisde the main for loop

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
  free(work);
  return 0;
}
