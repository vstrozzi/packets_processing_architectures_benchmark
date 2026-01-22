# Packets Processing Architectures Benchmark

## Overview

This repository contains the code and results from the Bachelor Thesis "A practical comparison of packets' processing architectures" by Virgilio Strozzi (September 2022).

The project investigates and compares different packet processing architectures to address the growing challenge of network speeds increasing faster than CPU frequencies. 

## Abstract

Accelerating the processing of packets is becoming increasingly important in recent years, due to the faster growth of the network speed compared to CPU frequencies. Multiple techniques to cope with this problem have been developed, using different programming paradigms and positioning in the processing-route of a packet. Each of these solutions can have different strengths and weaknesses on the development and the execution of different workloads, due to differences in the implementation and architectural design. 

We investigate the behaviour and performances of different packets' processing architectures (C Sockets, eBPF-XDP, DPDK and PsPIN) on different meaningful network-workloads (Data Aggregation, Copying from/to the Host, Data Filtering, Data Histogram, Key-Value store and Data Reduction) using different setups and different packet's sizes.  We also analyse the performances of the above architectures when executed directly on a Linux Server or on a specialized Data Processing Unit (DPU), such as the NVIDIA BlueField-2. 

The result of this experiment is a comparison of different measurements (RTT, Internal Elapsed Time and CPU load) where some architectures perform better than others depending on the setting.  We found that PsPIN produces the higher throughput among all other architectures in our setup. We also suggest some improvements on similar future work to provide a more rigorous statistical analysis.

## Architectures Tested

The following packet processing architectures were benchmarked (using perf tool):

- **C Sockets** - Traditional socket-based networking
- **eBPF-XDP** (Extended Berkeley Packet Filter - eXpress Data Path) - In-kernel packet processing
- **DPDK** (Data Plane Development Kit) - Kernel-bypass framework
- **PsPIN** (Processing in Smart Network Interface Cards) - Hardware-accelerated processing
- **NVIDIA BlueField-2 DPU** - Specialized Data Processing Unit for offloading packet processing


## Original Work

This work is part of the SPCL (Scalable Parallel Computing Lab) group at ETH Zurich. 

**Original repository**: https://spclgitlab.ethz.ch/explore/
