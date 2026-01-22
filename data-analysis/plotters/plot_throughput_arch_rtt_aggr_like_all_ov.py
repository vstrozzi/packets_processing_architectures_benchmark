import matplotlib.pyplot as plt
import numpy as np


SIZES = [1024, 512, 64]
FILE_SIZE = 1048576 # 1MiB

labels = ['aggregate', 'cp_from_h', 'cp_to_host', 'histog', 'reduce', 'filter']
fname = ['aggregate', 'copy_from_host', 'copy_to_host', 'histogram', 'reduce', 'filter']
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
            data_loc = []
            file = open(set + "/" + arch + "_" + lab + "_ben_rtt.txt", "r")
            # Read benchmark for given size
            j = 0
            for size in SIZES:
                chunks_nr = 100
                # Compute Throughput
                time = 0
                for i in range(0, chunks_nr):
                    j += 1
                    time += int(file.readline())
                    # Compute throughput in Gbit/s (substract overhead call time)
                data_loc.append(round((8*FILE_SIZE*100)/(time), 1))

            data_tot.append(data_loc)
            file.close()
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
ax[0, 0].set_title('C-Socket Host (overhead clock_gettime())')
ax[0, 0].set_xticks(x, labels)
ax[0, 0].set_ylim([0, 3.2])
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
ax[0, 1].set_title('eBPF-XDP Host (overhead clock_gettime())')
ax[0, 1].set_xticks(x, labels)
ax[0, 1].set_ylim([0, 3.2])
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
ax[0, 2].set_title('DPDK Host (overhead clock_gettime())')
ax[0, 2].set_xticks(x, labels)
ax[0, 2].set_ylim([0, 3.2])
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
ax[1, 0].set_title('C Socket BlueField2 (overhead clock_gettime())')
ax[1, 0].set_xticks(x, labels)
ax[1, 0].set_ylim([0, 3.2])
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
ax[1, 1].set_title('eBPF-XDP BlueField2 (overhead clock_gettime())')
ax[1, 1].set_xticks(x, labels)
ax[1, 1].set_ylim([0, 3.2])
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
ax[1, 2].set_title('DPDK BlueField2 (overhead clock_gettime())')
ax[1, 2].set_xticks(x, labels)
ax[1, 2].set_ylim([0, 3.2])
ax[1, 2].legend()

for rect in rects:
    ax[1, 2].bar_label(rect, fontsize=6, padding=3)



fig.tight_layout()

plt.show()