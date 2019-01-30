#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Reads a trace file produced by valgrind and an address marker file produced
 * by the program being traced. Outputs only the memory reference lines in
 * between the two markers
 */

int main(int argc, char **argv) {
    
    if(argc != 3) {
         fprintf(stderr, "Usage: %s tracefile markerfile\n", argv[0]);
         exit(1);
    }


    // Addresses should be stored in unsigned long variables
    unsigned long start_marker, end_marker;


    // Here I set up the markers
    FILE *fp = fopen(argv[2], "r+");

    if (fp == NULL){
	printf("FILE FP NOT FOUND");
    }
	
    fscanf(fp, "%lx %lx", &start_marker, &end_marker);

    // Here I manage the trace file, and print every line inside the trace file.
    FILE *fp_2 = fopen(argv[1], "r+");
    if (fp_2 == NULL){
	printf("FILE FP2 NOT FOUND");
    }
    char alphabet;
    long address;
    char comma;
    int number;
    int to_print = 0;
   
    while (fscanf(fp_2, "%c %lx %*c %*d ", &alphabet, &address, &comma, &number) != EOF){
	if (address == end_marker){
		to_print = 0;
	}
	if (to_print == 1){
		printf("%c,%#lx\n", alphabet, address);
	}
           //print block goes here
	if (address == start_marker){
		to_print = 1;
	} 

    }
    /* For printing output, use this exact formatting string where the
     * first conversion is for the type of memory reference, and the second
     * is the address
     */
    // 
    fclose(fp);
    fclose(fp_2);
    return 0;
}

