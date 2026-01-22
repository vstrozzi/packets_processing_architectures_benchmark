#!/bin/bash

clang -O2 -g -Wall -target bpf -c fw_back_ov.c -o fw_back_ov.o

gcc fw_back_ov_loader.c -o fw_back_ov_loader -lbpf -lxdp

gcc fw_back_ov_stats.c -o fw_back_ov_stats