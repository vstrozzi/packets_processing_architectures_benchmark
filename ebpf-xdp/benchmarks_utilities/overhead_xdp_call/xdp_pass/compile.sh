#!/bin/bash

clang -O2 -g -Wall -target bpf -c xdp_pass.c -o xdp_pass.o

gcc xdp_pass_loader.c -o xdp_pass_loader -lbpf -lxdp

gcc xdp_pass_stats.c -o xdp_pass_stats