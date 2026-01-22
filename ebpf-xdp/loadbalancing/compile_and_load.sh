#!/bin/bash

clang -O2 -g -Wall -target bpf -c loadbalancing.c -o loadbalancing.o

gcc loadbalancing_loader.c -o loadbalancing_loader -lbpf -lxdp
./loadbalancing_loader veth1
