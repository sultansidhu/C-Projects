#!/usr/bin/env bash

gcc -Wall -std=gnu99 -g echo_arg.c
./a.out csc209 > echo_out.txt
gcc -Wall -std=gnu99 -g echo_stdin.c
./a.out < echo_stdin.c
gcc -Wall -std=gnu99 -g count.c
./a.out 210 | wc -m
gcc -Wall -std=gnu99 -g echo_stdin.c
ls -l -S | head -2 | tail -1 | cut -d " " -f 9 | ./a.out
