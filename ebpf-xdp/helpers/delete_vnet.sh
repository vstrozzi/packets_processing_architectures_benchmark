#!/bin/bash

ip route del 10.0.1.0/32 dev veth1
ip link delete veth1
ip netns delete vnet0
