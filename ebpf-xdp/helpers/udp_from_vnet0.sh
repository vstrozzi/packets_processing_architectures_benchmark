#!/bin/bash

ip netns exec vnet0 nc -u 10.0.3.0 7999
