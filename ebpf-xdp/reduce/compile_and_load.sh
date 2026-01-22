#!/bin/bash

clang -O2 -g -Wall -target bpf -c reduce.c -o reduce.o

gcc reduce_loader.c -o reduce_loader -lbpf -lxdp
./reduce_loader enp4s0f1np1