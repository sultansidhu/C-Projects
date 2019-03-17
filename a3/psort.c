#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "helper.h"

// checker and wrapper functions

void check_usage(){
  fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
  exit(1);
}

FILE * Fopen(char * filename, char * mode){
  FILE * fp = fopen(filename, mode);
  if (fp == NULL){
    fprintf(stderr, "fopen failure; no such file exists\n");
    exit(1);
  }
  return fp;
}

struct rec get_minimum_struct(struct rec * record_array, int num_processes){
  struct rec minimum = record_array[0];
  for (int i = 0; i < num_processes; i++){
    if (compare_freq(&(record_array[i]), &(minimum)) < 0){
      minimum = record_array[i];
    }
  }
  return minimum;
}

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

void Pipe(int * fd){
  if ((pipe(fd))==-1){
    perror("pipe");
    exit(1);
  }
}

void fill_child_unsorted_array(int i, int * work, int num_records_inside_process,
  struct rec * records, struct rec * unsorted_records){
  int sum = 0;
  for (int t = 0; t < i; t++){
    sum += work[t];
  }

  for (int x = 0; x < num_records_inside_process; x++){
    records[x] = unsorted_records[sum];
    sum++;
  }
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

void set_file_pt_position(FILE *fp, int position, int *work_array){
  int sum = 0;
  for (int i = 0; i < position; i++){
    sum += work_array[i];
  }
  fseek(fp, sum * sizeof(struct rec), SEEK_SET);
}

int main(int argc, char* argv[]){

  printf("CHECK MALLOC YOU FUCK \n");
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
  printf("ERROR: WRONG FLAGS AREN'T POINTED OUT, FIX IT\n");

  // get file size using helper

  int filesize = get_file_size(filename);
  int num_entries = filesize / sizeof(struct rec);

  // decide how much work to delegate to each process

  int * work = malloc(sizeof(int) * num_processes); // how much work per child
  struct rec * unsorted_records = malloc(filesize); // the array of unsorted records
  struct rec * sorted_records = malloc(filesize); // array of the sorted records

  printf("FREE THIS\n");
  delegate_work(work, num_processes, num_entries);

  // fill up the array of records in heap memory

  FILE * fp = Fopen(filename, "rb");
  FILE * out = Fopen(output, "wb");
  struct rec temporary_record;
  for (int j = 0; j < num_entries; j++){
    fread(&temporary_record, sizeof(struct rec), 1, fp);
    unsorted_records[j] = temporary_record;
  }
  printf("CHECK IF THIS CODE BLOCK IS NEEDED.\n");
  printf("In this new design the child doesn't read, but takes in the unsorted records from\n");
  // read(fd[i][0], &(temp[i]), sizeof(struct rec));
  // printf("temp[i] contains %s with freq %d\n", temp[i].word, temp[i].freq);

  // create the pipes that will be used for communication

  int fd[num_processes-1][2]; // a pipe per child process
  struct rec temp[num_processes-1];

  // The master for loop begins

  for (int i = 0; i < num_processes; i++){

    // convert file descriptor subarray to pipe

    Pipe(fd[i]);

    // call fork to obtain a child process

    int result;
    if ((result = Fork()) == 0){

      // read data

      // set_file_pt_position(fp, i, work);
      // printf("this function doesnt work^, use gdb\n");
      struct rec * records = malloc(sizeof(struct rec) * work[i]);
      int num_records_inside_process = work[i];

      // store in array (malloc'd)

      // for (int k = 0; k < num_records_inside_process; k++){
      //   fread(records, sizeof(struct rec), 1, fp);
      // }

      fill_child_unsorted_array(i, work, num_records_inside_process, records, unsorted_records);

      // printf("in child process now, printing contents: \n");
      // for (int m = 0; m < num_records_inside_process; m++){
      //   printf("the entry in child number %d is: %s\n", i, records[m].word);
      // }

      // call qsort on it

      qsort(records, num_records_inside_process, sizeof(struct rec), compare_freq);

      // printf("AFTER SORTING \n\n");
      // for (int m = 0; m < num_records_inside_process; m++){
      //   printf("the sorted entry in child number %d is: %s\n", i, records[m].word);
      // }

      // start feeding into the pipe

      close(fd[i][0]); // close reading end
      for (int a = 0; a < num_records_inside_process; a++){
        write(fd[i][1], &(records[a]), sizeof(struct rec));
        //printf("the entry is %s\n", records[a].word);
      }
      close(fd[i][1]); // close writing end after writing is done
      free(records);
      exit(0);


    } else {

      // close unneeded writing ends

      close(fd[i][1]); // closes the writing end

      // run wait() as many times as the children

      int exit_sig;
      int result = Wait(&exit_sig);

      // reading it into the reading ends of the pipe

      read(fd[i][0], &(temp[i]), sizeof(struct rec));
      // printf("temp[i] contains %s with freq %d\n", temp[i].word, temp[i].freq);
      printf("FIGURE OUT WHAT TO PRINT WHEN END OF PIPE IS REACHED FOR ONE PIPE AND NOT FOR THE OTHER\n");

    }  // else block
  } // for loop block

  // merge the results together

  // take the minimum from temp and then dump it into the output file

  printf("code block directly below doesn't work\n");
  for (int h = 0; h < num_entries; h++){
    struct rec minimum = get_minimum_struct(temp, num_processes);
    printf("figure out how to use dup2 here to write the struct to the file\n");
    printf("code doesn't work because fwrite is called instead of write\n");
    fwrite(out, sizeof(struct rec), 1, out);
  }

  // close the reading ends of the fd

  for (int k = 0; k < num_processes; k++){
    close(fd[k][0]); //closing reading ends after the program is done.
  }

  // free all malloc'd space

  free(work);
  free(sorted_records);
  free(unsorted_records);
  fclose(fp);
  fclose(out);
  return 0;
} // end of main
