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

  int * work = malloc(sizeof(int) * num_processes);
  printf("FREE THIS\n");
  delegate_work(work, num_processes, num_entries);

  

}
