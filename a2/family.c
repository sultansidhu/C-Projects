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

static int counter = 0;
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
    fam.signature = malloc(sizeof(char) * (strlen(str)+1));
    strncpy(fam.signature, str, strlen(str));
    fam.signature[strlen(str)] = '\0';
    fam.max_words=family_increment;
    fam.word_ptrs=malloc(sizeof(char *) * (family_increment + 1));
    fam.word_ptrs[family_increment] = NULL; // newly added
    *fam_pt = fam;
    return fam_pt;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) { // MIGHT HAVE FIXED THE PROBLEM //TODO LOOK AT DIS
    if (fam->max_words == fam->num_words){
        char **temp_ptr = realloc(fam->word_ptrs, (fam->max_words + family_increment)*(sizeof(char *)));
        fam->word_ptrs = temp_ptr;
        //fam->word_ptrs=realloc(fam->word_ptrs, (unsigned long)(fam->max_words) + family_increment);
        fam->max_words = fam->max_words + family_increment;
        fam->word_ptrs[fam->max_words-1] = NULL;//newly added
    }
    fam->word_ptrs[fam->num_words] = word;
    fam->num_words++;
    counter++;
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
        free(current_fam_pt->signature);
        current_fam_pt = current_fam_pt->next;
    }
    Family *fam_pt = fam_list;
    while (fam_pt!= NULL){
        Family *next = fam_pt->next;
        free(fam_pt);
        fam_pt = next;
    }
}


/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
    //initialize variables
    int i = 0;
    Family *fam_linked_list = NULL;

    //iterate through word_list
    while(word_list[i] != NULL){
        char *word = word_list[i];
        char signature[strlen(word_list[i])+1];
        strncpy(signature, word, strlen(word_list[i]));
        for (int j = 0; j < strlen(word_list[i]); j++){
            if (signature[j] != letter){
                signature[j] = '-';
            }
        }
        signature[strlen(word_list[i])-1] = '\0';
        // signature made, now search for family with that signature
        Family *sigs_fam = find_family(fam_linked_list, signature);
        if (sigs_fam == NULL){
            //family not found, create new family
            Family *new_fam = new_family(signature);
            add_word_to_family(new_fam, word);
            new_fam -> next = fam_linked_list;
            fam_linked_list = new_fam;
        } else {
            //printf("#####delete this#####\n");
            //printf("sigs fam has num words %d\n", sigs_fam->num_words);
            //int m = 0;
           // while(sigs_fam->word_ptrs[m] != NULL){
               // m++;
            //}
            //printf("sizeof sigs fam-> wordsptrs is %d\n", m);
            //printf("######## delete till here#####\n");
            // TODO: REMEMBER TO SET ARRAYS ENDS TO NULL, WHICH IS PROBABLY CAUSING THIS PROBLEM6, wherever you see "//newly added", the feature has been accommodated.
            //family found, add word to family
            add_word_to_family(sigs_fam, word);
        }
        i++;
    }
    return fam_linked_list;
}


/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) {
    //char *copy = malloc(sizeof(char) * strlen(fam->signature)); // free
    //strncpy(copy, (*fam).signature, strlen((*fam).signature));
    return fam->signature;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
    char **new_array = malloc(sizeof(char *) * (fam->num_words+1));
    for (int i = 0; i < fam->num_words; i++){
        // TODO: PROBLEM IS THAT THE NUM WORDS IS SUPPOSED TO BE EQUAL TO NUMBER OF THINGS IN WORD_PTRS
        // TODO: DONT MALLOC THE INDIVIDUAL THINGS INSIDE, ONLY THE NEW ARRAY ITSELF IS MALLOC'D
        char *newptr = ((fam->word_ptrs)[i]);
        new_array[i] = newptr;
    }
    new_array[fam->num_words] = NULL;
    // put last index as a null
    return new_array;
}


/* Return a pointer to a random word from fam.
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
    int random = rand();
    random = random % fam->num_words;
    return fam->word_ptrs[random];
}
