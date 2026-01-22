#!/bin/bash


# add route through tap (ip is ficticious)
ip route add 10.0.3.0/24 dev tap0
# add arp entry with MAC address of some fictious eth address
arp -s 10.0.3.0 52:54:00:27:fc:68

