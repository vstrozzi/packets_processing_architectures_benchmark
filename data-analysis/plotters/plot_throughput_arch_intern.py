import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

labels = ['aggregate', 'cp_from_h', 'cp_to_host', 'histog', 'reduce', 'filter', "pingpong", "kvstore"]
fname = ['aggregate', 'copy_from_host', 'copy_to_host', 'histogram', 'reduce', 'filter', 'pingpong', 'kvstore']
archs = ['host', 'bl2']
archs_name = ['Host', 'BlueField2']
cases = ['ov', 'no_ov']
overhead_clk = [1111.84, 50.84]
overhead_dpdk = [29.20, 22.14]
overhead_bpf = [1101.41, 48.68]
data = []
netw_arch = "pspin"

# Get raw_data from loc files
v = 0
for arch in archs:
    for case in cases:
        data_tot = []
        for lab in fname:
            j = 0
            data_loc = []
            file = open("baseline/" + arch + "_" + lab + "_ben_proc.txt", "r")
            # Read benchmark for given size
            for size in SIZES:
                values = np.empty(32768)
                chunks_nr = 32768
                # Compute ThroughputW
                time = np.longlong(0)
                for k in range (0, chunks_nr):
                    j += 1
                    val = file.readline()

                    if case == 'no_ov':
                        time += int(val) - overhead_bpf[v]
                        values[k] = int(val) - overhead_bpf[v]
                    else:
                        time += int(val)
                        values[k] = (val)
                    # Compute throughput in Gbit/s (substract overhead call time)
                if size == 1024:
                    #data_loc.append(round(32*FILE_SIZE*8/(time), 2))
                    data_loc.append(round(1024*8/(np.median(values)), 2))
                elif size == 512:
                    #data_loc.append(round(16*FILE_SIZE*8/(time), 2))
                    data_loc.append(round(512*8/(np.median(values)), 2))
                elif size == 64:
                    #data_loc.append(round(2*FILE_SIZE*8/(time), 2))
                    data_loc.append(round(64*8/(np.median(values)), 2))

            data_tot.append(data_loc)
            file.close()

        data.append(data_tot)
    v += 1
fig, ax = plt.subplots(2, 2)

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


print(data_real)

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
ax[0, 0].set_title('C Socket Host Median (overhead rte_rdtsc_precise() and rte_get_tsc_hz())')
ax[0, 0].set_xticks(x, labels)
ax[0, 0].set_ylim([0, 65])
ax[0, 0].legend()

for rect in rects:
    ax[0, 0].bar_label(rect, fontsize=7, padding=3)

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
    rects.append(ax[1, 0].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 0].set_ylabel('Gbit/s')
ax[1, 0].set_title('C Socket Host Median (no overhead rte_rdtsc_precise() and rte_get_tsc_hz())')
ax[1, 0].set_xticks(x, labels)
ax[1, 0].set_ylim([0, 65])
ax[1, 0].legend()

for rect in rects:
    ax[1, 0].bar_label(rect, fontsize=7, padding=3)

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
    rects.append(ax[0, 1].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 1].set_ylabel('Gbit/s')
ax[0, 1].set_title('C Socket BlueField2 Median (overhead rte_rdtsc_precise() and rte_get_tsc_hz())')
ax[0, 1].set_xticks(x, labels)
ax[0, 1].set_ylim([0, 65])
ax[0, 1].legend()

for rect in rects:
    ax[0, 1].bar_label(rect, fontsize=7, padding=3)

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
    rects.append(ax[1, 1].bar(base + step*count, data_real[count], width, label=str(size) + " B", color=colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 1].set_ylabel('Gbit/s')
ax[1, 1].set_title('C Socket BlueField2 Median (no overhead rte_rdtsc_precise() and rte_get_tsc_hz())')
ax[1, 1].set_xticks(x, labels)
ax[1, 1].set_ylim([0, 65])
ax[1, 1].legend()

for rect in rects:
    ax[1, 1].bar_label(rect, fontsize=7, padding=3)


fig.tight_layout()

plt.show()