#!/bin/bash

gcc xdp_no_pass_loader.c -o xdp_no_pass_loader -lbpf -lxdp

gcc xdp_no_pass_stats.c -o xdp_no_pass_stats