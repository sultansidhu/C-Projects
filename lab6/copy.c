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
  int len_dest = sizeof(dest) / sizeof(char);
  if (capacity >= len_dest){
    // more space needed
    for (int i = 0; i < len_dest-1 ; i++){
      dest[i] = src[i];
    }
    dest[len_dest-1] = '\0';
  } else {
    // space suffices
    for (int i = 0; i < capacity; i++){
      dest[i] = src[i];
    }
    for (int j = capacity; j<len_dest; j++){
      dest[j] = '\0';
    }
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
