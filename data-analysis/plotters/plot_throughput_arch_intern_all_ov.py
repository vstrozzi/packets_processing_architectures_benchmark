import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

labels = ["1024B", "512B", "64B"]
settings = ["C Socket Host", "eBPF-XDP Host", "DPDK Host", "C Socket Bl2", "eBPF-XDP Bl2", "DPDK Bl2", "PsPIN"]
fname = ['aggregate', 'copy_from_host', 'copy_to_host', 'histogram']
sets = ["baseline", "ebpf-xdp", "dpdk"]
archs = ['host', 'bl2']
archs_name = ['Host', 'BlueField2']
cases = ['ov', 'no_ov']
overhead_clk = [1111.84, 50.84]
overhead_dpdk = [29.20, 22.14]
overhead_bpf = [1101.41, 48.68]
data = []

# Get raw_data from loc files

overheads = [overhead_clk, overhead_bpf, overhead_dpdk]

# Get raw_data from loc files
v = 0
for lab in fname:
    u = 0
    data_tot = []
    for arch in archs:
        s = 0
        for set in sets:
            j = 0
            data_loc = []
            file = open(set + "/" + arch + "_" + lab + "_ben_proc.txt", "r")
            print(set + "/" + arch + "_" + lab + "_ben_proc.txt", "r")
            # Read benchmark for given size
            for size in SIZES:
                chunks_nr = 32768
                # Compute ThroughputW
                time = np.longlong(0)
                for k in range (0, chunks_nr):
                    j += 1   
                    time += int(file.readline()) # - overheads[s][u]
                    # Compute throughput in Gbit/s (substract overhead call time)
                if time < 0:
                    data_loc.append(0)
                    continue
                if size == 1024:
                    data_loc.append(round(32*FILE_SIZE*8/(time), 1))
                elif size == 512:
                    data_loc.append(round(16*FILE_SIZE*8/(time), 1))
                elif size == 64:
                    data_loc.append(round(2*FILE_SIZE*8/(time), 1))
            data_tot.append(data_loc)
            file.close()
            s +=1
        u += 1
    file = open("pspin/pspin_throughput.txt", "r")
    data_loc = []
    # Read benchmark for given size
    for h in range(0, v):
            for size in SIZES:
                file.readline()#

    for size in SIZES:
        values = np.empty(32768)
        chunks_nr = 1
        # Compute ThroughputW
        
        val = float(file.readline().split(" ")[0])

            
        data_loc.append(round((val), 1))

    data_tot.append(data_loc)
   
    data.append(data_tot)
    v += 1

print(len(data))
print(data)
fig, ax = plt.subplots(2, 2)
# Create data array
data_real = data[0]
x = np.arange(len(labels))  # the label locations
width = 0.10  # the width of the bars

# Creates rects
count = 0
step = width
base = x - width  - 0.20
rects = []
colors = ["darkblue", "royalblue", "deepskyblue"]
for set in settings:
    rects.append(ax[0, 0].bar(base + step*count, data_real[count], width, label=set))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 0].set_ylabel('Gbit/s')
ax[0, 0].set_title('Aggregate')
ax[0, 0].set_xticks(x, labels)
ax[0, 0].set_ylim([0, 95])
ax[0, 0].legend()

for rect in rects:
    ax[0, 0].bar_label(rect, fontsize=6, padding=3)


data_real = data[1]
x = np.arange(len(labels))  # the label locations
width = 0.10  # the width of the bars

# Creates rects
count = 0
step = width
base = x - width  - 0.20
rects = []
colors = ["darkblue", "royalblue", "deepskyblue"]
for set in settings:
    rects.append(ax[0, 1].bar(base + step*count, data_real[count], width, label=set))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 1].set_ylabel('Gbit/s')
ax[0, 1].set_title('Copy From Host')
ax[0, 1].set_xticks(x, labels)
ax[0, 1].set_ylim([0, 85])
ax[0, 1].legend()

for rect in rects:
    ax[0, 1].bar_label(rect, fontsize=6, padding=3)


#create data array
data_real = data[2]
x = np.arange(len(labels))  # the label locations
width = 0.10  # the width of the bars

# Creates rects
count = 0
step = width
base = x - width  - 0.20
rects = []
colors = ["darkblue", "royalblue", "deepskyblue"]
for set in settings:
    rects.append(ax[1, 0].bar(base + step*count, data_real[count], width, label=set))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 0].set_ylabel('Gbit/s')
ax[1, 0].set_title('Copy To Host')
ax[1, 0].set_xticks(x, labels)
ax[1, 0].set_ylim([0, 95])
ax[1, 0].legend()

for rect in rects:
    ax[1, 0].bar_label(rect, fontsize=6, padding=3)


data_real = data[3]
x = np.arange(len(labels))  # the label locations
width = 0.10  # the width of the bars

# Creates rects
count = 0
step = width
base = x - width  - 0.20
rects = []
colors = ["darkblue", "royalblue", "deepskyblue"]
for set in settings:
    rects.append(ax[1, 1].bar(base + step*count, data_real[count], width, label=set))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 1].set_ylabel('Gbit/s')
ax[1, 1].set_title('Histogram')
ax[1, 1].set_xticks(x, labels)
ax[1, 1].set_ylim([0, 95])
ax[1, 1].legend()

for rect in rects:
    ax[1, 1].bar_label(rect, fontsize=6, padding=3)
plt.show()