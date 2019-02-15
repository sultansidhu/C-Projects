#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Write the copy function to perform exactly as strncpy does, with one
   exception: your copy function will guarantee that dest is always
   null-terminated.
   You shoud read the man page to learn how strncpy works.

  NOTE: You must write this function without using any string functions.
  The only function that should depend on string.h is memset.
 */

char *copy(char *dest, const char *src, int capacity) {
  if (capacity >= (sizeof(src) / sizeof(char))){
    int j = 0;
    while(src[j] != '\0'){
      dest[j] = src[j];
      if (j == capacity){
        break;
      }
      j++;
    }
    dest[j] = '\0';
  } else {
    for (int i = 0; i < capacity; i++){
      dest[i] = src[i];
    }
    dest[capacity] = '\0';
  }
    return dest;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: copy size src\n");
        exit(1);
    }
    int size = strtol(argv[1], NULL, 10);
    char *src = argv[2];

    char dest[size];
    memset(dest, 'x', size);

    copy(dest, src, size);

    printf("%s\n", dest);
    return 0;
}
