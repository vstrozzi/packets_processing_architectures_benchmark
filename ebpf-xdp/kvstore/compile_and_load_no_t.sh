#!/bin/bash

clang -O2 -g -Wall -target bpf -c kvstore_no_t.c -o kvstore_no_t.o

gcc kvstore_loader_no_t.c -o kvstore_loader_no_t -lbpf -lxdp

./kvstore_loader_no_t enp4s0f1np1
