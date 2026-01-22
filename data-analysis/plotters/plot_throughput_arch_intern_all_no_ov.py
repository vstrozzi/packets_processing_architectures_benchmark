import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

labels = ['aggr', 'cp_f_h', 'cp_t_h', 'histog', 'red', 'filt', "pingp", "kvst"]
fname = ['aggregate', 'copy_from_host', 'copy_to_host', 'histogram', 'reduce', 'filter', 'pingpong', 'kvstore']
sets = ["baseline", "ebpf-xdp", "dpdk"]
archs = ['host', 'bl2']
archs_name = ['Host', 'BlueField2']
cases = ['ov', 'no_ov']
overhead_clk = [1111.84, 50.84]
overhead_bpf = [1101.41, 48.68]
overhead_dpdk = [29.20, 22.14]

overheads = [overhead_clk, overhead_bpf, overhead_dpdk]
data = []

# Get raw_data from loc files
v = 0
for arch in archs:
    u = 0
    for set in sets:
        data_tot = []
        for lab in fname:
            j = 0
            data_loc = []
            file = open(set + "/" + arch + "_" + lab + "_ben_proc.txt", "r")
            # Read benchmark for given size
            for size in SIZES:
                chunks_nr = 32768
                # Compute ThroughputW
                time = np.longlong(0)
                for k in range (0, chunks_nr):
                    j += 1   
                    if lab == "pingpong" and arch == "host":
                        time += int(file.readline()) - overheads[u][v]
                    else:
                        time += int(file.readline()) - overheads[u][v]
                    # Compute throughput in Gbit/s (substract overhead call time)
                if size == 1024:
                    data_loc.append(round(32*FILE_SIZE*8/(time), 1))
                elif size == 512:
                    data_loc.append(round(16*FILE_SIZE*8/(time), 1))
                elif size == 64:
                    data_loc.append(round(2*FILE_SIZE*8/(time), 1))
            data_tot.append(data_loc)
            file.close()
        u += 1
        data.append(data_tot)
    v += 1
fig, ax = plt.subplots(2, 3)

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
    rects.append(ax[0, 0].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 0].set_ylabel('Gbit/s')
ax[0, 0].set_title('C-Socket Host (no overhead clock_gettime())')
ax[0, 0].set_xticks(x, labels)
ax[0, 0].set_ylim([0, 105])
ax[0, 0].legend()

for rect in rects:
    ax[0, 0].bar_label(rect, fontsize=6, padding=3)

data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[1][j][i])
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
    rects.append(ax[0, 1].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 1].set_ylabel('Gbit/s')
ax[0, 1].set_title('eBPF-XDP Host (no overhead bpf_ktime_get_ns())')
labels_1 = ['aggr', 'cp_f_h', 'cp_t_h', 'histog', 'red', 'filt', "pingp*", "kvst"]
ax[0, 1].set_xticks(x, labels_1)
ax[0, 1].set_ylim([0, 105])
ax[0, 1].legend()

for rect in rects:
    ax[0, 1].bar_label(rect, fontsize=6, padding=3)

data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[2][j][i])
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
    rects.append(ax[0, 2].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 2].set_ylabel('Gbit/s')
ax[0, 2].set_title('DPDK Host (no overhead rte_rdtsc_precise()/rte_get_tsc_hz())')
ax[0, 2].set_xticks(x, labels)
ax[0, 2].set_ylim([0, 105])
ax[0, 2].legend()

for rect in rects:
    ax[0, 2].bar_label(rect, fontsize=6, padding=3)

data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[3][j][i])
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
    rects.append(ax[1, 0].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 0].set_ylabel('Gbit/s')
ax[1, 0].set_title('C Socket BlueField2 (no overhead clock_gettime())')
ax[1, 0].set_xticks(x, labels)
ax[1, 0].set_ylim([0, 105])
ax[1, 0].legend()

for rect in rects:
    ax[1, 0].bar_label(rect, fontsize=6, padding=3)


data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[4][j][i])
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
    rects.append(ax[1, 1].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 1].set_ylabel('Gbit/s')
ax[1, 1].set_title('eBPF-XDP BlueField2 (no overhead bpf_ktime_get_ns())')
ax[1, 1].set_xticks(x, labels)
ax[1, 1].set_ylim([0, 105])
ax[1, 1].legend()

for rect in rects:
    ax[1, 1].bar_label(rect, fontsize=6, padding=3)

data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[5][j][i])
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
    rects.append(ax[1, 2].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 2].set_ylabel('Gbit/s')
ax[1, 2].set_title('DPDK BlueField2 (no overhead rte_rdtsc_precise()/rte_get_tsc_hz())')
ax[1, 2].set_xticks(x, labels)
ax[1, 2].set_ylim([0, 105])
ax[1, 2].legend()

for rect in rects:
    ax[1, 2].bar_label(rect, fontsize=6, padding=3)



fig.tight_layout()

plt.show()