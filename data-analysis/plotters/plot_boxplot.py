import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.patches as mpatches
from pathlib import Path

# Sizes to send
SIZES = [1024, 512, 64] 
FILE_SIZE = 1048576 # 1MiB
NAMES = ["eBPF-XDP", "DPDK", "C Socket"]
colors = ["darkblue", "royalblue", "deepskyblue"]
# Get parent path
path = Path.cwd().resolve()
path_par = Path.cwd().parent.resolve()


# Get name of benchmark files
fname = [d for d in path.iterdir() if
       d.is_dir() and
       (str(d.name).startswith("baseline") or 
       str(d.name).startswith("dpdk") or
       str(d.name).startswith("ebpf-xdp"))]

print(len(fname))

fname = [f for k in fname for f in k.iterdir() if
        str(f.name).endswith(".txt")
        and (str(f.name).startswith("host_") or 
        str(f.name).startswith("bl2_")) and
        "time" not in str(f.name) and
        "cpu" not in str(f.name) and
        "overhead" not in str(f.name)]

print(len(fname))

fname.sort()
# Set label


# Get raw_data from loc files
num = 0
for lab in fname:
    data = []
    file = open(str(lab), "r")
    print(str(lab))
    # Read benchmark for given 
    for size in SIZES:
        data_loc = []
        if "rtt" in lab.name and ("pingpong" in lab.name or "kvstore" in lab.name) :
            chunks_nr = 102400
        elif "rtt" in lab.name:
            chunks_nr = 100
        else:
            chunks_nr = 32768
        print(chunks_nr)
        # Compute Throughput
        for i in range(0, chunks_nr):
            # print(time)
            #if time > 100000:
             #   print(time)
             #   continue
            # Compute throughput in Gbit/s (substract overhead call time)
            data_loc.append(int(file.readline())/1000000)
        data.append(data_loc)
    file.close()

    pd_dat = pd.DataFrame({"1024B Size": data[0],
                       "512B Size": data[1],
                       "64B Size": data[2]})

    print(pd_dat)
    # Create data array
    #print(data_real)
    #print(len(data_real))
    # Set up the plot
    # Draw the plot
    fig = plt.figure()
    for i in range(0, 3):
        ax = plt.subplot(1, 3, i + 1)
        sns.boxplot(data=pd_dat.iloc[:, i], width=.5, linewidth=1.2,
                    fliersize=3, whis = 1.5, color = colors[i], medianprops=dict(color="yellow", alpha=0.7))

        # Title and labels
        ax.set_xlabel(str(SIZES[i]) + "B", size = 11)
        ax.set_ylabel('Delay (ms)', size= 11)
        #ax.set_xlim([0, 2*data.mean()])
        ax.grid(axis='y')
        ax.legend(loc=0)

        handles, labels = ax.get_legend_handles_labels()

    
        empty_patch = mpatches.Patch(color='none', label="Mean value: " + str(pd_dat.iloc[i].mean())) 
        handles.append(empty_patch)
        labels.append("Mean value: " + str(round(pd_dat.iloc[i].mean(), 4)))
        
        ax.legend(handles, labels, loc = "upper right", fontsize=4)

    name = str(lab.name).split("_")
    if name[1] == "copy":
        conc = ' '.join([name[1], name[2], name[3]])
        copy = []
        copy.append(name[0])
        copy.append(conc)
        for i in range(4, len(name)):
            copy.append(name[i])
        name = copy
    for j in range(0, len(name)):
        name[j] = name[j].capitalize()
    val = 0 if num < 32 else (1 if num < 64 else 2) 
    if name[0] =="Bl2":
        name[0] = "BlueField2"
    if name[3] == "Rtt.txt":
        fig.suptitle("Latency per Packet (1 MiB): " +
                      name[0] + " (" + NAMES[val] + ")" + " on application " + name[1] + " (RTT)", size = 9)
    else:
        fig.suptitle("Latency per Packet (1 MiB):    " +
                      name[0] + " (" + NAMES[val] + ")" + " on application " + name[1] + " (Internal)", size = 9)
    plt.tight_layout()
    
    plt.savefig('plot/box_plots/' + NAMES[val] + "_" + str(lab.name).replace(".txt", "") + '.png', dpi=500)
    num += 1



