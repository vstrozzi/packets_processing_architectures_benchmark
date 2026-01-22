#!/bin/bash

# send data of some fixed length
ip netns exec vnet0 nping --udp -p 7999 10.0.3.0 --data-length 1024
