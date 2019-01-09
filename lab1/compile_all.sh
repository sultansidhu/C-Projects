#!/usr/bin/env bash
gcc -Wall -std=gnu99 -g -o HALLO hello.c
./HALLO
gcc -Wall -std=gnu99 -g -o echo_arg echo_arg.c
./echo_arg Something for the input
gcc -Wall -std=gnu99 -g -o echo_stdin echo_stdin.c
./echo_stdin 
gcc -Wall -std=gnu99 -g -o count count.c
./count 10
