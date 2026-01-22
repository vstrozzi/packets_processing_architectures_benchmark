import math
import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


ns = [1024, 2048, 16384]

zs = [2.3263]
intervals = []
labels = ['aggregate', 'cp_from_h', 'cp_to_host', 'filter', 'histog', 'reduce', "pingpong", "kvstore"]
fname = ['aggregate', 'copy_from_host', 'copy_to_host', 'filter', 'histogram','reduce', 'pingpong', 'kvstore']
archs = ['bl2', 'host']
archs_name = ['Host', 'BlueField2']
setups = ["dpdk", "ebpf-xdp", "baseline"]
overhead_clk = [1111.84, 50.84]
overhead_dpdk = [29.20, 22.14]
overhead_bpf = [1101.41, 48.68]
data = []

SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

# Compute intervals
for n in ns:
    for z in zs:
        intervals.append(math.floor((n - z*math.sqrt(n))/2))
        intervals.append(math.ceil((n + z*math.sqrt(n))/2 + 1))

# Compute confidence intervals for every application, for every size
v = 0
for arch in archs:
    print()
    for set in setups:
        print()
        data_tot = []
        for lab in fname:
            print("We have {0} on {1} for {2}".format(arch, set, lab))
            j = 0
            data_loc = []
            file = open(set + "/" + arch + "_" + lab + "_ben_proc.txt", "r")
            # Read benchmark for given size
            values_med = []
            o = 0
            print()
            for size in SIZES:
                values = np.empty(FILE_SIZE//size)
                chunks_nr = FILE_SIZE//size
                # Compute ThroughputW
                time = np.longlong(0)
                for k in range (0, chunks_nr):
                    j += 1
                    val = file.readline()
                    values_med.append(int(val))
                
                values_med.sort()
                # Depending on chunks_nr, compute bounds of interval
                for z in range(0, 1):
                    b1 = values_med[int(intervals[2*o])]
                    b2 = values_med[int(intervals[2*o + 1])]
                    if o != 2:
                        print("[" + str(b1) + ", " +str(b2) + "]", end=", \\newline")
                        continue
                    print("[" + str(b1) + ", " +str(b2) + "]", end="")

                    # Compute throughput in Gbit/s (substract overhead call time)
                o += 1
            print()
            file.close()

    v += 1
