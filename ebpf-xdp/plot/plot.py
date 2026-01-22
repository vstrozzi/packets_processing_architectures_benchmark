import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

labels = ['ktime_call', 'baseline', 'aggregate', 'cp_from_h', 'cp_to_host', 'histog', 'reduce', 'filter']
fname = ['ktime_call', 'baseline', 'aggregate', 'copy_from_host', 'copy_to_host', 'histogram', 'reduce', 'filter']
data = []

# Get raw_data from loc files
for lab in fname:
    data_loc = []
    file = open(lab + "_ben.txt", "r")
    # Read benchmark for given size
    for size in SIZES:
        file.readline()
        chunks_nr = FILE_SIZE//size
        # Compute Throughput
        time = 0
        for i in range(0, chunks_nr):
            time += int(file.readline())
        # Compute throughput in Gbit/s (substract overhead call time)
        data_loc.append(round((FILE_SIZE*8)/(time), 1))
        
    data.append(data_loc)
    file.close()

# Create data array
data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[j][i])
    data_real.append(data_loc)


x = np.arange(len(labels))  # the label locations
width = 0.30  # the width of the bars

fig, ax = plt.subplots()

# Creates rects
count = 0
step = width
base = x - width
rects = []
for size in SIZES:
    rects.append(ax.bar(base + step*count, data_real[count], width, label=str(size) + " B"))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Gbit/s')
ax.set_title('Application')
ax.set_xticks(x, labels)
ax.legend()

for rect in rects:
    ax.bar_label(rect, padding=3)

fig.tight_layout()

plt.show()