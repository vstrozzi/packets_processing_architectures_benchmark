import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024]
FILE_SIZE = 1048576 # 1MiB

labels = ['Aggregate', "Ping Pong"]
fname = ['aggregate', 'pingpong']
data = []

# Get raw_data from loc files
for lab in fname:
    j = 0
    data_loc = []
    file = open("baseline/" + "bl2_" + lab + "_cpu.txt", "r")
    # Read benchmark for given size
    for size in SIZES:
        chunks_nr = 410
        # Compute ThroughputW
        values = []
        for k in range (0, chunks_nr):
                
            val = file.readline().replace('\n', '')
            values.append(int(val)/1000000)
            # Compute throughput in Gbit/s (substract overhead call time)
        data_loc.append(values)
    data.append(data_loc)
    file.close()

# Create data array
data_real = data

x = np.arange(len(labels))  # the label locations
width = 0.30  # the width of the bars

x = list(np.arange(0, 41, 0.1))
fig, ax = plt.subplots(1, 2)
k = 0
for j in range (0, 2):
    ax[j].plot(x, data[k][0])
    ax[j].grid()
    ax[j].set(xlabel='Time (s)', ylabel=' Cpu Cycles ($10^6$)', title = str(labels[k]))
    ax[j].set_ylim([0, 450])
    ax[j].get_lines()[0].set_color("royalblue")
    k += 1



plt.show()