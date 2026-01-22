#!/bin/bash

clang -O2 -g -Wall -target bpf -c ping_pong_no_t.c -o ping_pong_no_t.o

gcc ping_pong_loader_no_t.c -o ping_pong_loader_no_t -lbpf -lxdp

./ping_pong_loader_no_t enp4s0f1np1
