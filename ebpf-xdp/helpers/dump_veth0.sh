#!/bin/bash

sudo ip netns exec vnet0 tcpdump -i veth0 -l -A
