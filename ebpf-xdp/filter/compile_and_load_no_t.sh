#!/bin/bash

clang -O2 -g -Wall -target bpf -c filter_no_t.c -o filter_no_t.o

gcc filter_loader_no_t.c -o filter_loader_no_t -lbpf -lxdp
./filter_loader_no_t enp4s0f1np1
