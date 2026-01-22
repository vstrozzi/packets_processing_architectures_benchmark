#!/bin/bash


# create veth
ip link add veth0 type veth peer name veth1

# create vnet0
ip netns add vnet0
ip link set veth0 netns vnet0
ip -n vnet0 addr add 10.0.1.0/32 dev veth0
ip -n vnet0 link set veth0 up
ip -n vnet0 link set lo up

# configure local veth1
ip addr add 10.0.3.0/32 dev veth1
ip link set veth1 up

# set both routing table
ip -n vnet0 route add 10.0.3.0/32 dev veth0
ip route add 10.0.1.0/32 dev veth1


