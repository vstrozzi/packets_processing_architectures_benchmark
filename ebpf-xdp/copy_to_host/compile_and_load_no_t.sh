#!/bin/bash

clang -O2 -g -Wall -target bpf -c copy_to_host_no_t.c -o copy_to_host_no_t.o

gcc copy_to_host_loader_no_t.c -o copy_to_host_loader_no_t -lbpf -lxdp
./copy_to_host_loader_no_t enp4s0f1np1
