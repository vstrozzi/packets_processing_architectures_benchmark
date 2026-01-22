import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import seaborn as sns
from pathlib import Path

# Sizes to send
SIZES = [1024, 512, 64] 
FILE_SIZE = 1048576 # 1MiB
chunks_nr = 32768
# Get parent path
path = Path.cwd().resolve()
path_par = Path.cwd().parent.resolve()


# Get name of benchmark files
fname = [d for d in path.iterdir() if
       d.is_dir() and
       str(d.name).startswith("dpdk")][0]



fname = [f for f in fname.iterdir() if
        str(f.name).endswith(".txt")]

fname.sort()
# Set label
labels = [l.name.replace("_ben_proc.txt", "") for l in fname]
print(labels)
labels.sort()
data = []
data_real = []
# Get raw_data from loc files
for lab in fname:
    if not lab.name.startswith("host_pingpong_ben_proc.txt"):
        continue
    file = open(str(lab), "r")
    # Read benchmark for given 
    for size in SIZES:
        data_loc = []
        data_loc_real = []
        # Compute Throughput
        for i in range(0, chunks_nr):
            time = int(file.readline())
            data_loc_real.append(time)
            # print(time)
            if time > 3000:
                print(time)
                continue
            # Compute throughput in Gbit/s (substract overhead call time)
            data_loc.append(time)

        data.append(data_loc)
        data_real.append(data_loc_real)
    file.close()
    break


data = np.asarray(data)
data_real = np.asarray(data_real)
print(data_real.mean())
print(data.size)
print(data_real.size)
# Create data array
#print(data_real)
#print(len(data_real))
# Set up the plot
ax = plt.subplot(2, 3, 1)
# Draw the plot

sns.distplot(data[0], hist=True, kde=True, 
            bins=800 , color = 'darkblue', 
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1})

# Title and labels
ax.set_title("1 MiB in 1024B packets", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Probability Density Function', size= 11)
ax.set_xlim([0, 2*np.asarray(data[0]).mean()])
ax.axvline(data_real[0].mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data_real[0].mean()))
ax.axvline(np.median(data_real[0]), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data_real[0])))
ax.plot([], color="w", label="Std {:.2f}".format(data_real[0].std()))
ax.grid(axis='y')
ax.legend(loc=0)

ax = plt.subplot(2, 3, 4)
sns.distplot(data[0], hist=True, kde=True,
            bins=800, 
            hist_kws={'cumulative': True},
            kde_kws= {'cumulative': True}
            )
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Cumulative Density Function', size= 11)
ax = plt.subplot(2, 3, 2)
# Draw the plot

sns.distplot(data[1], hist=True, kde=True, 
            bins=800 , color = 'darkblue', 
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1})

# Title and labels
ax.set_title("1 MiB in 512B packets", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Probability Density Function', size= 11)
ax.set_xlim([0, 2*np.asarray(data[1]).mean()])
ax.axvline(data_real[1].mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data_real[1].mean()))
ax.axvline(np.median(data_real[1]), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data_real[1])))
ax.plot([], color="w", label="Std {:.2f}".format(data_real[1].std()))
ax.grid(axis='y')
ax.legend(loc=0)

ax = plt.subplot(2, 3, 5)
sns.distplot(data[1], hist=True, kde=True,
            bins=800, 
            hist_kws={'cumulative': True},
            kde_kws= {'cumulative': True}
            )

ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Cumulative Density Function', size= 11)

ax = plt.subplot(2, 3, 3)


# Draw the plot
sns.distplot(data[2], hist=True, kde=True, 
            bins=800 , color = 'darkblue', 
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1})

# Title and labels
ax.set_title("1 MiB in 64B packets", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Probability Density Function', size= 11)
ax.set_xlim([0, 2*np.asarray(data[2]).mean()])
ax.axvline(data_real[2].mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data_real[2].mean()))
ax.axvline(np.median(data[2]), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data_real[2])))
ax.plot([], color="w", label="Std {:.2f}".format(data_real[2].std()))
ax.grid(axis='y')
ax.legend(loc=0)

ax = plt.subplot(2, 3, 6)
sns.distplot(data[2], hist=True, kde=True,
            bins=800, 
            hist_kws={'cumulative': True},
            kde_kws= {'cumulative': True}
            )

ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Cumulative Density Function', size= 11)
plt.tight_layout()
plt.show()
