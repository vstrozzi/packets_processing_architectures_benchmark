#!/bin/bash

clang -O2 -g -Wall -target bpf -c histogram.c -o histogram.o

gcc histogram_loader.c -o histogram_loader -lbpf -lxdp
./histogram_loader enp4s0f1np1