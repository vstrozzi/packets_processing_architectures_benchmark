#!/bin/bash

clang -O2 -g -Wall -target bpf -c ping_pong.c -o ping_pong.o

gcc ping_pong_loader.c -o ping_pong_loader -lbpf -lxdp

./ping_pong_loader enp4s0f1np1
