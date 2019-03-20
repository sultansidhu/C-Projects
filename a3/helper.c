#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
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

/*Delegates the amount of work that would be needed to be performed by each of
of the child processes.*/
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

/*Calculates the number of bytes to skip before the reading from the file
starts for each of the children.*/
int get_bytes_to_skip(int iteration, int * work){
  int sum = 0;
  for (int i = 0; i < iteration; i++){
    sum += work[i];
  }
  return (sum * sizeof(struct rec));
}

/*Goes through an array of structs, and loads the struct with the lowest freq,
and the index of it into the the pointers passed in.*/
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

/*Gets the size of the binary file from which the records are being read.*/
int get_file_size(char *filename) {
    struct stat sbuf;

    if ((stat(filename, &sbuf)) == -1) {
       perror("stat");
       exit(1);
    }

    return sbuf.st_size;
}

/* A comparison function to use for qsort */
int compare_freq(const void *rec1, const void *rec2) {

    struct rec *r1 = (struct rec *) rec1;
    struct rec *r2 = (struct rec *) rec2;

    if (r1->freq == r2->freq) {
        return 0;
    } else if (r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}
