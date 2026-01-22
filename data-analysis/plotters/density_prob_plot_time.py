import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from pathlib import Path

# Sizes to send
SIZES = [1024] 
FILE_SIZE = 1048576 # 1MiB

# Get parent path
path = Path.cwd().resolve()
path_par = Path.cwd().parent.resolve()


# Get name of benchmark files
fname = [d for d in path.iterdir() if
       d.is_dir() and
       str(d.name).startswith("ebpf-xdp")][0]

fname = [f for f in fname.iterdir() if
        str(f.name).endswith("bl2_time_overhead.txt")]

fname.sort()
# Set label
labels = [l.name.replace("_ben_proc.txt", "") for l in fname]
print(labels)
labels.sort()
data = []
data_real = []
# Get raw_data from loc files
for lab in fname:
    file = open(str(lab), "r")
    # Read benchmark for given 
    for size in SIZES:
        data_loc = []
        chunks_nr = 1000000
        # Compute Throughput
        for i in range(0, chunks_nr):
            time = int(file.readline())
            # print(time)
            # Compute throughput in Gbit/s (substract overhead call time)
            data_real.append(time)
            if time > 2000:
               continue
            data_loc.append(time)

        data.append(data_loc)
    file.close()
    break

# Create data array

# Set up the plot


# Draw the plot

ax = plt.subplot(2, 1, 1)
data = np.asarray(data)
data_real = np.asarray(data_real)
print(len(data[0]))
sns.distplot(data, hist=True, kde=True,
            bins=400, color = 'darkblue', 
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1}
            )

# Title and labels
ax.set_title("$10^6$ Measurement of rte_rdtsc_precise() and rte_get_tsc_hz()", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Probabilty Density Function', size= 11)
ax.set_xlim([0, 60])
ax.axvline(data_real.mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data_real.mean()))
ax.axvline(np.median(data_real), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data_real)))
ax.plot([], color="w", label="Std {:.2f}".format(data_real.std()))
ax.legend(loc=0)

ax = plt.subplot(2, 1, 2)
print(len(data[0]))
sns.distplot(data, hist=True, kde=True,
            bins=100, 
            hist_kws={'cumulative': True},
            kde_kws= {'cumulative': True}
            )

# Title and labels
ax.set_title("$10^6$ Measurement of rte_rdtsc_precise() and rte_get_tsc_hz()", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Cumulative Density Function', size= 11)
ax.set_xlim([0, 60])
ax.axvline(data_real.mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data_real.mean()))
ax.axvline(np.median(data_real), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data_real)))
ax.plot([], color="w", label="Std {:.2f}".format(data_real.std()))
ax.legend(loc=0)



plt.tight_layout()
plt.show()
