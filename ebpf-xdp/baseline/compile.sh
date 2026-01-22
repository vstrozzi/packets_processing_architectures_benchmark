#!/bin/bash

clang -O2 -g -Wall -target bpf -c baseline.c -o baseline.o

gcc baseline_loader.c -o baseline_loader -lbpf -lxdp
