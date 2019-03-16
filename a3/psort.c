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

int Fork(){
  int result = fork();
  if (result < 0){
    perror("fork");
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
      case 'f':
        filename = optarg;
      case 'o':
        output = optarg;
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

  FILE * fp = fopen(filename, "rb");
  struct rec temporary_record;
  for (int j = 0; j < num_entries; j++){
    fread(&temporary_record, sizeof(struct rec), 1, fp);
    unsorted_records[j] = temporary_record;
  }
  printf("CHECK IF THIS CODE BLOCK IS NEEDED.\n");

  // create the pipes that will be used for communication

  int fd[num_processes-1][2]; // a pipe per child process

  // The master for loop begins

  for (int i = 0; i < num_processes; i++){

    // convert file descriptor subarray to pipe

    Pipe(fd[i]);

    // call fork to obtain a child process

    int result;
    if ((result = Fork()) == 0){

      // read data

      set_file_pt_position(fp, i, work);
      printf("this function doesnt work^, use gdb")
      struct rec * records = malloc(sizeof(struct rec) * 2);
      printf("FREE THIS\n");
      int num_records_inside_process = work[i];

      // store in array (possibly malloc'd)

      for (int k = 0; k < num_records_inside_process; k++){
        fread(records, sizeof(struct rec), 1, fp);
      }

      // printf("in child process now, printing contents: \n");
      // for (int m = 0; m < num_records_inside_process; m++){
      //   printf("the entry is: %s\n", records[m].word);
      // }

      // call qsort on it

      qsort(records, num_records_inside_process, sizeof(struct rec), compare_freq);

      // printf("AFTER SORTING \n\n");
      // for (int m = 0; m < num_records_inside_process; m++){
      //   printf("the entry is: %s\n", records[m].word);
      // }

    } else {
      // parent process
    }
  }



}
