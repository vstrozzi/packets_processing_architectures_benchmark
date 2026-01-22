import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

labels = ['aggregate', 'cp_from_h', 'cp_to_host', 'histog', 'reduce', 'filter', "pingpong"]
fname = ['aggregate', 'copy_from_host', 'copy_to_host', 'histogram', 'reduce', 'filter', 'pingpong']
archs = ['host', 'bl2']
archs_name = ['Host', 'BlueField2']
cases = ['ov', 'no_ov']
overhead_clk = [1111.84, 50.84]
overhead_dpdk = [29.20, 22.14]
overhead_bpf = [1101.41, 48.68]
data = []

# Get raw_data from loc files
v = 0
data_tot = []
file = open("pspin/pspin_throughput.txt", "r")
for lab in fname:
    j = 0
    data_loc = []
    # Read benchmark for given size
    for size in SIZES:
        values = np.empty(32768)
        chunks_nr = 1
        # Compute ThroughputW
        
        val = float(file.readline().split(" ")[0])

            
        data_loc.append(round((val), 2))

    data_tot.append(data_loc)
file.close()

data_tot.append([410, 400, 52])
data.append(data_tot)
v += 1
ax = plt.gca()


labels.append("kvstore*")
fname.append("kvstore")

print(data)
print(len(data))
# Create data array
data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[0][j][i])
    data_real.append(data_loc)

x = np.arange(len(labels))  # the label locations
width = 0.30  # the width of the bars



# Creates rects
count = 0
step = width
base = x - width
rects = []
colors = ["darkblue", "royalblue", "deepskyblue"]
for size in SIZES:
    rects.append(ax.bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Gbit/s')
ax.set_title('PsPIN Cycle Accurate Simulation')
ax.set_xticks(x, labels)
ax.legend()

for rect in rects:
    ax.bar_label(rect, fontsize=7, padding=3)




plt.show()