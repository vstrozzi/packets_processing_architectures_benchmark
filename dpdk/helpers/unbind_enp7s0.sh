#!/bin/bash

# Unbind interface 00:09.0
dpdk-devbind.py --unbind 0000:06:00.0

# Rebind to driver e1000
echo -n "0000:06:00.0" > /sys/bus/pci/drivers/e1000e/bind

# Put interface up
ifconfig enp0s6 up



