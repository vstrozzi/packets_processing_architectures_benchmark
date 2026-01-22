#!/bin/bash

# Unbind interface 00:08.0
dpdk-devbind.py --unbind 0000:08:00.0

# Rebind to driver e1000
echo -n "0000:08:00.0" > /sys/bus/pci/drivers/e1000e/bind

# Put interface up
ifconfig enp8s0 up


