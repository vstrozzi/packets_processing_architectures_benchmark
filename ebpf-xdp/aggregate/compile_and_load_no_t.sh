#!/bin/bash

clang -O2 -g -Wall -target bpf -c aggregate_no_t.c -o aggregate_no_t.o

gcc aggregate_loader_no_t.c -o aggregate_loader_no_t -lbpf -lxdp
./aggregate_loader_no_t enp4s0f1np1
