#!/bin/bash


# Abilitate IOMMU on VM
echo 1 > /sys/module/vfio/parameters/enable_unsafe_noiommu_mode
# Take down the interface to be able to bind it
ifconfig p1 down
# Bind interface to new driver to comm with DPDK
dpdk-devbind.py --bind=vfio-pci p1


