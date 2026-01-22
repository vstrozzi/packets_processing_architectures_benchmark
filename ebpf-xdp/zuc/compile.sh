#!/bin/bash

clang -O2 -g -Wall -target bpf -c zuc.c -o zuc.o

gcc zuc_loader.c -o zuc_loader -lbpf -lxdp

gcc zuc_stats.c -o zuc_stats
