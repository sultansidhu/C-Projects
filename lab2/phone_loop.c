#include <stdio.h>
#include <stdlib.h>

void main(){
  char phone[11];
  int number;
  scanf("%s %i", &phone, &number);
  int result = phone_func(phone, number);
  while(1 > 0){
    scanf("%i", &number);
    if (number == EOF){
      break;
    }
    result = phone_func(phone, number);
  }
}

int phone_func(char *char_array, int number){
  if (number == -1){
    for (int i = 0; i < 10; i++){
      printf("%c", char_array[i]);
    }
    printf("\n");
    return 0;
  } else if (number < 10 && number > -1){
    printf("%c\n", char_array[number]);
    return 0;
  } else {
    printf("ERROR\n");
    return 1;
  }
}
