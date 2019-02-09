#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "family.h"

/* Number of word pointers allocated for a new family.
   This is also the number of word pointers added to a family
   using realloc, when the family is full.
*/
static int family_increment = 0;


/* Set family_increment to size, and initialize random number generator.
   The random number generator is used to select a random word from a family.
   This function should be called exactly once, on startup.
*/
void init_family(int size) {
    family_increment = size;
    srand(time(0));
}


/* Given a pointer to the head of a linked list of Family nodes,
   print each family's signature and words.

   Do not modify this function. It will be used for marking.
*/
void print_families(Family* fam_list) {
    int i;
    Family *fam = fam_list;

    while (fam) {
        printf("***Family signature: %s Num words: %d\n",
               fam->signature, fam->num_words);
        for(i = 0; i < fam->num_words; i++) {
            printf("     %s\n", fam->word_ptrs[i]);
        }
        printf("\n");
        fam = fam->next;
    }
}


/* Return a pointer to a new family whose signature is
   a copy of str. Initialize word_ptrs to point to
   family_increment+1 pointers, numwords to 0,
   maxwords to family_increment, and next to NULL.
*/
Family *new_family(char *str) {
  //char **words[family_increment+1]
  Family *fam_pt = malloc(sizeof(Family));
  Family fam;
    fam.num_words=0;
    fam.next=NULL;
    strncpy(fam.signature, str, strlen(str));
    fam.max_words=family_increment;
    fam.word_ptrs=malloc(sizeof(char *) * (family_increment + 1));
  *fam_pt = fam;
  return fam_pt;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) {
  if (fam->max_words == fam->num_words){
    fam->word_ptrs=realloc(fam->word_ptrs, fam->max_words+family_increment);
    fam->max_words = fam->max_words + family_increment;
  }
  fam->word_ptrs[fam->num_words] = word;
  fam->num_words++;
  // check if dereferencing is needed within the function
}


/* Return a pointer to the family whose signature is sig;
   if there is no such family, return NULL.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_family(Family *fam_list, char *sig) {
  Family *current_fam_pt = fam_list;
  while (current_fam_pt != NULL){
    if (strcmp(current_fam_pt->signature, sig)==0){
      break;
    } else {
      current_fam_pt = current_fam_pt->next;
    }
  }
  return current_fam_pt;
}


/* Return a pointer to the family in the list with the most words;
   if the list is empty, return NULL. If multiple families have the most words,
   return a pointer to any of them.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_biggest_family(Family *fam_list) {
  Family *current_fam_pt = fam_list;
  int max = 0;
  Family *max_fam;
  while (current_fam_pt != NULL){
    if (current_fam_pt->num_words > max){
      max = current_fam_pt->num_words;
      max_fam = current_fam_pt;
    }
    current_fam_pt = current_fam_pt->next;
  }
  return max_fam;
}


/* Deallocate all memory rooted in the List pointed to by fam_list. */
void deallocate_families(Family *fam_list) {
  Family *current_fam_pt = fam_list;
  while (current_fam_pt != NULL){
    free(current_fam_pt->word_ptrs);
    current_fam_pt = current_fam_pt->next;
  }
  free(fam_list);
}


/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
  // 1. Initialize a temporary array of created families, will hold them before linking
  Family *temp_fams = malloc(sizeof(Family)); // FREE THIS
  int len_temp = 0;
  // 2. iterate over the word list
  int i = 0;
  while (word_list[i] != NULL){
    // 3. For every word, generate its signature depending on the letter
    char *signature = malloc(sizeof(char)*strlen(word_list[i])); // FREE THIS
    strncpy(signature, word_list[i], strlen(word_list[i]));
    // modify the word_copy
    for (int j = 0; j < strlen(signature); j++){
      if (signature[j] != letter){
        signature[j] = '-'; // FIGURE OUT HOW TO PRESERVE SIGNATURES COMING FROM PREVIOUS TRIES
      }
    }
    // now signature is a signature
    // 4. See if the family with that signature exists within the created array,
    // 4(a) if it does then add word to that family
    // 4(b) if it does not, then create new family and add that word
    if (len_temp != 0){
      for (int k = 0; k < len_temp; k++){
        Family fomlay = temp_fams[k];
        if (strcmp(get_family_signature(&fomlay), signature) == 0){
          // if family found
          add_word_to_family(&fomlay, word_list[i]);
        } else {
          // if family not found then make new family
          Family *fam = new_family(signature);
          add_word_to_family(fam, word_list[i]);
          temp_fams[len_temp] = *fam;
          len_temp++;
        }
      }

    } else {
      // create new family, because there are none currently
      Family *fam2 = new_family(signature);
      add_word_to_family(fam2, word_list[i]);
      temp_fams[len_temp] = *fam2;
      len_temp++;
    }
    i++;
  }
  // 5. Link the families together
  for (int m = 0; m < len_temp-1; m++){
    temp_fams[m].next = &temp_fams[m+1];
  }
  temp_fams[len_temp-1].next = NULL;

  // 6. Return the linked list
  return temp_fams;
}


/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) { // ADD A FREE FOR THIS
  char *copy = malloc(sizeof(char) * strlen(fam->signature)); // free
  strncpy(copy, (*fam).signature, strlen((*fam).signature));
  return copy;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
  // malloc these boys
  char **new_array = malloc(sizeof(char *) * fam->num_words);
  for (int i = 0; i < fam->num_words; i++){
    new_array[i] = (fam->word_ptrs)[i];
  }
  return new_array;
}


/* Return a pointer to a random word from fam.
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
  int random = rand();
  random = random % (*fam).num_words;
  return (*fam).word_ptrs[random];
}
