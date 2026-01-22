#!/bin/bash

clang -O2 -g -Wall -target bpf -c aggregate.c -o aggregate.o

gcc aggregate_loader.c -o aggregate_loader -lbpf -lxdp
./aggregate_loader enp4s0f1np1
