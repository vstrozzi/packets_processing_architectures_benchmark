#!/bin/bash

# Execute it after setting up dpdk dependecies

# Create tap (to communicate with program)
./build/loadbalancing -l 0 --vdev=net_tap0,iface=tap0
