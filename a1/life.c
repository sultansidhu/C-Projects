#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_state(char *state, int size);
void update_state(char *state, int size);


int main(int argc, char **argv) {

    if (argc != 3) {
    	fprintf(stderr, "Usage: USAGE: life initial n\n");
    	return 1;
    }

    int size = strlen(argv[1]);
    char *state = argv[1];
    int repetitions = strtol(argv[2], NULL, 10);
    for (int i = 0;i < repetitions; i++){
      print_state(state, size);
      update_state(state, size);
    }
    return 0;


}

