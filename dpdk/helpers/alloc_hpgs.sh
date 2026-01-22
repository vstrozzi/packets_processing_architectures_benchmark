#!/bin/bash

# Alloc hugepages (64 for node0 of size 2048kB)
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge
echo 10000 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages


