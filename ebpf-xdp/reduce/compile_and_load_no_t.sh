#!/bin/bash

clang -O2 -g -Wall -target bpf -c reduce_no_t.c -o reduce_no_t.o

gcc reduce_loader_no_t.c -o reduce_loader_no_t -lbpf -lxdp
./reduce_loader_no_t enp4s0f1np1