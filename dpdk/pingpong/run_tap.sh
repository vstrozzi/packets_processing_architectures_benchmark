#!/bin/bash

# Execute it after setting up dpdk dependecies

# Create tap (to communicate with program)
./build/pingpong -l 0-3 --bind=vfio-pci p1
