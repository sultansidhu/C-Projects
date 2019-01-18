#include <stdio.h>
#include <stdlib.h>

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



int main(){
  char phone[11];
  int number;
  scanf("%s %i", &phone, &number);
  phone_func(phone, number);
}
