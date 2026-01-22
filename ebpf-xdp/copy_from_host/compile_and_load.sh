#!/bin/bash

clang -O2 -g -Wall -target bpf -c copy_from_host.c -o copy_from_host.o

gcc copy_from_host_loader.c -o copy_from_host_loader -lbpf -lxdp
./copy_from_host_loader enp4s0f1np1
