#!/bin/bash

# Execute it after setting up dpdk dependecies

# Create tap (to communicate with program)
./build/copy_to_host -l 0-3 --vdev=net_tap0,iface=tap0
