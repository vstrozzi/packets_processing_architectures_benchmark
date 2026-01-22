#!/bin/bash

clang -O2 -g -Wall -target bpf -c kvstore.c -o kvstore.o

gcc kvstore_loader.c -o kvstore_loader -lbpf -lxdp

./kvstore_loader enp4s0f1np1
