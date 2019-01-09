#!/usr/bin/env bash

gcc -Wall -std=gnu99 -g echo_arg.c
./a.out csc209 > echo_out.txt
gcc -Wall -std=gnu99 -g echo_stdin.c
./a.out | ./a.out given input
gcc -Wall -std=gnu99 -g count.c
./a.out 210 | wc | cut -d "1" -f 2
gcc -Wall -std=gnu99 -g echo_stdin.c
ls -l -S | cut -d " " -f 14 | head -3 | tail -1 | ./a.out
