#!/bin/bash

clang -O2 -g -Wall -target bpf -c filter.c -o filter.o

gcc filter_loader.c -o filter_loader -lbpf -lxdp
./filter_loader enp4s0f1np1
