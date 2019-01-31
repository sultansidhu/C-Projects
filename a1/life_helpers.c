#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_state(char *state, int size){
  for (int i = 0; i < size; i++){
    printf("%c", state[i]);
  }
  printf("\n");
}
void update_state(char *state, int size){
  char new_state[size];
  new_state[0] = state[0];
  new_state[size-1] = state[size-1];
  for (int i = 1; i < size-1;i+=1){
    char neighbour_left = state[i-1];
    char neighbour_right = state[i+1];
    if (neighbour_left == neighbour_right){
      new_state[i] = '.';
    }else{
      new_state[i] = 'X';
    }
  }
  for (int i = 0; i < size; i++){
    state[i] = new_state[i];
  }

}
