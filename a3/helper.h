#ifndef _HELPER_H
#define _HELPER_H

#define SIZE 44

struct rec {
    int freq;
    char word[SIZE];
};

int get_file_size(char *filename);
int compare_freq(const void *rec1, const void *rec2);
void check_usage();
FILE * Fopen(char * filename, char * mode);
int Close(int file_des);
int Fork();
int Wait(int * exit_sig);
int Fwrite(void * ptr, size_t size, size_t nitems, FILE * stream);
void Pipe(int * fd);
void * Malloc(size_t size);
int Write(int fd, void * pointer, size_t size);
int Read(int file_des, void * buffer, size_t count, struct rec * ifempty);
int Fclose(FILE * fd);
void delegate_work(int work[], int num_processes, int num_entries);
int get_bytes_to_skip(int iteration, int * work);
void get_minimum_struct(struct rec * rec_ptr, int * index_ptr, struct rec * buffer, int num_processes);
int get_file_size(char * filename);
int compare_freq(const void * rec1, const void * rec2);

#endif /* _HELPER_H */
