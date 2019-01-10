#!/usr/bin/env bash

./echo_arg csc209 > echo_out.txt
./echo_stdin < echo_stdin.c
./count 210 | wc -m
ls -l -S | head -2 | tail -1 | cut -d " " -f 9 | ./echo_stdin
