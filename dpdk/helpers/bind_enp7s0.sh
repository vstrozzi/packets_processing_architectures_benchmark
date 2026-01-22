#!/bin/bash

# Take down the interface to be able to bind it
ifconfig enp4s0f1np1 down
# Bind interface to new driver to comm with DPDK
dpdk-devbind.py --bind=vfio-pci enp4s0f1np1

