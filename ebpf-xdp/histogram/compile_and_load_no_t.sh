#!/bin/bash

clang -O2 -g -Wall -target bpf -c histogram_no_t.c -o histogram_no_t.o

gcc histogram_loader_no_t.c -o histogram_loader_no_t -lbpf -lxdp
./histogram_loader_no_t enp4s0f1np1