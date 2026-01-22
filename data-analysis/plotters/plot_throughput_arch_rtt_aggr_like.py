import chunk
from email.mime import base
import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

labels = ['aggregate', 'cp_from_h', 'cp_to_host', 'histog', 'reduce', 'filter']
fname = ['aggregate', 'copy_from_host', 'copy_to_host', 'histogram', 'reduce', 'filter']
data = []
archs = ['host', 'bl2']
archs_name = ['Host', 'BlueField2']
cases = ['ov', 'no_ov']

# Get raw_data from loc files
v = 0
for arch in archs:
    for case in cases:
        data_tot = []
        for lab in fname:
            data_loc = []
            file = open("ebpf-xdp/" + arch + "_" + lab + "_ben_rtt.txt", "r")
            # Read benchmark for given size
            j = 0
            for size in SIZES:
                chunks_nr = 100
                # Compute Throughput
                time = 0
                for i in range(0, chunks_nr):
                    j += 1
                    if case == 'no_ov':
                        time += int(file.readline()) - 1111.84
                    else:
                        time += int(file.readline())
                    # Compute throughput in Gbit/s (substract overhead call time)
                data_loc.append(round((8*FILE_SIZE*100)/(time), 4))

            data_tot.append(data_loc)
            file.close()
        data.append(data_tot)
    v += 1
fig, ax = plt.subplots(2, 2)


# Create data array
data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[0][j][i])
    data_real.append(data_loc)


x = np.arange(len(labels))  # the label locations
width = 0.30  # the width of the bars

# Speed up over overhead
o = 0
thr_ov = 0
thr_no_ov = 0
for case in data:
    if o % 2 == 0:
        for app in case:
            for val in app:
                thr_ov += val
    else:
        for app in case:
            for val in app:
                thr_no_ov += val
    o += 1
print(thr_ov)
print(thr_no_ov)
print("Throughput ration ov: " + str((1 - thr_ov/thr_no_ov)*100))
# Speed up Bl2 over host
o = 0
thr_host_no_ov = 0
thr_bl2_no_ov = 0
for case in data:
    if o % 4 == 0:
        for app in case:
            for val in app:
                thr_host_no_ov += val
    elif o % 4 == 2:
        for app in case:
            for val in app:
                thr_bl2_no_ov += val
    o += 1
print(thr_host_no_ov)
print(thr_bl2_no_ov)
print("Throughput ration bl2: " + str((1 - thr_bl2_no_ov/thr_host_no_ov)*100))


# Creates rects
count = 0
step = width
base = x - width
rects = []
colors = ["darkblue", "royalblue", "deepskyblue"]
for size in SIZES:
    rects.append(ax[0, 0].bar(base + step*count, data_real[count], width, label=str(size) + " B", color = colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 0].set_ylabel('Gbit/s')
ax[0, 0].set_title('eBPF-XDP Host (overhead of clock_gettime())')
ax[0, 0].set_xticks(x, labels)
ax[0, 0].set_ylim([0, 2.2])
ax[0, 0].legend()

for rect in rects:
    ax[0, 0].bar_label(rect, fontsize=7, padding=3)


# Create data array
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
    rects.append(ax[1, 0].bar(base + step*count, data_real[count], width, label=str(size) + " B", color = colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 0].set_ylabel('Gbit/s')
ax[1, 0].set_title('eBPF-XDP Host (no overhead of clock_gettime())')
ax[1, 0].set_xticks(x, labels)
ax[1, 0].set_ylim([0, 2.2])
ax[1, 0].legend()

for rect in rects:
    ax[1, 0].bar_label(rect, fontsize=7, padding=3)


# Create data array
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
    rects.append(ax[0, 1].bar(base + step*count, data_real[count], width, label=str(size) + " B", color = colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[0, 1].set_ylabel('Gbit/s')
ax[0, 1].set_title('eBPF-XDP BlueField2 (overhead of clock_gettime())')
ax[0, 1].set_xticks(x, labels)
ax[0, 1].set_ylim([0, 2.2])
ax[0, 1].legend()

for rect in rects:
    ax[0, 1].bar_label(rect, fontsize=7, padding=3)


# Create data array
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
    rects.append(ax[1, 1].bar(base + step*count, data_real[count], width, label=str(size) + " B", color = colors[count]))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax[1, 1].set_ylabel('Gbit/s')
ax[1, 1].set_title('eBPF-XDP BlueField2 (no overhead of clock_gettime())')
ax[1, 1].set_xticks(x, labels)
ax[1, 1].set_ylim([0, 2.2])
ax[1, 1].legend()

for rect in rects:
    ax[1, 1].bar_label(rect, fontsize=7, padding=3)

fig.tight_layout()

plt.show()