#!/bin/bash

clang -O2 -g -Wall -target bpf -c ktime_call.c -o ktime_call.o

gcc ktime_call_loader.c -o ktime_call_loader -lbpf -lxdp
./ktime_call_loader veth1
