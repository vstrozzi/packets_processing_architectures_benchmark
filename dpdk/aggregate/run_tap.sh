#!/bin/bash

# Execute it after setting up dpdk dependecies

# Create tap (to communicate with program)
./build/aggregate -l 0-3 -a 04:00.1
