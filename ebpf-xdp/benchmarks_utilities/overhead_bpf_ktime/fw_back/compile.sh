#!/bin/bash

clang -O2 -g -Wall -target bpf -c fw_back.c -o fw_back.o

gcc fw_back_loader.c -o fw_back_loader -lbpf -lxdp

gcc fw_back_stats.c -o fw_back_stats