import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from pathlib import Path

# Sizes to send
SIZES = [1024, 512, 64] 
FILE_SIZE = 1048576 # 1MiB

# Get parent path
path = Path.cwd().resolve()
path_par = Path.cwd().parent.resolve()


# Get name of benchmark files
fname = [f for f in path.iterdir() if
         str(f.name).endswith(".txt")]
fname.sort()
# Set label
labels = [l.name.replace("_ben_proc.txt", "") for l in fname]
labels.sort()
data = []

# Get raw_data from loc files
for lab in fname:
    file = open(str(lab), "r")
    if lab.name != "aggregate":
        continue
    # Read benchmark for given size
    for size in SIZES:
        data_loc = []
        file.readline()
        chunks_nr = FILE_SIZE//size
        # Compute Throughput
        for i in range(0, chunks_nr):
            time = int(file.readline())
            # Compute throughput in Gbit/s (substract overhead call time)
            data_loc.append(time)
    
        data.append(data_loc)
    file.close()

# Create data array
data_real = data


# Set up the plot
ax = plt.subplot(2, 2, 1)
# Draw the plot
data_real[0] = [x for x in data_real[0] if x < 600]
data = np.asarray(data_real[0])
sns.distplot(data, hist=True, kde=True, 
            bins=100, color = 'darkblue', 
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1})

# Title and labels
ax.set_title("1 MiB in 1024B packets", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Density', size= 11)
ax.set_xlim([40, 500])
ax.axvline(data.mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data.mean()))
ax.axvline(np.median(data), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data)))
ax.axvline(data.std(), color='b', linestyle='dashed', linewidth=1, label="Std {:.2f}".format(data.std()))
ax.legend(loc=0)


# Set up the plot
ax = plt.subplot(2, 2, 2)
# Draw the plot
data_real[1] = [x for x in data_real[1] if x < 400]
data = np.asarray(data_real[1])
sns.distplot(data, hist=True, kde=True, 
            bins=100, color = 'darkblue', 
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1})

# Title and labels
ax.set_title("1 MiB in 512B packets", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Density', size= 11)
ax.set_xlim([50, 400])
ax.axvline(data.mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data.mean()))
ax.axvline(np.median(data), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data)))
ax.axvline(data.std(), color='b', linestyle='dashed', linewidth=1, label="Std {:.2f}".format(data.std()))
ax.legend(loc=0)


# Set up the plot
ax = plt.subplot(2, 2, 3)
data_real[2] = [x for x in data_real[2] if x < 400]
data = np.asarray(data_real[2])
# Draw the plot
sns.distplot(data, hist=True, kde=True,color = 'darkblue', 
            bins=200,
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1})

# Title and labels
ax.set_title("1 MiB in 64B packets", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Density', size= 11)
ax.set_xlim([0, 150])
ax.axvline(data.mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data.mean()))
ax.axvline(np.median(data), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data)))
ax.axvline(data.std(), color='b', linestyle='dashed', linewidth=1, label="Std {:.2f}".format(data.std()))
ax.legend(loc=0)


# Set up the plot
ax = plt.subplot(2, 2, 4)
data = [y for x in data_real for y in x]
data = np.asarray(data)
# Draw the plot
sns.distplot(data, hist=True, kde=True,color = 'darkblue', 
            bins=200,
            hist_kws={'edgecolor':'black'},
            kde_kws={'linewidth': 1})

# Title and labels
ax.set_title("1 MiB in all size-format", size = 11)
ax.set_xlabel('Delay (ns)', size = 11)
ax.set_ylabel('Density', size= 11)
ax.set_xlim([0, 500])
ax.axvline(data.mean(), color='k', linestyle='dashed', linewidth=1, label="Mean {:.2f}".format(data.mean()))
ax.axvline(np.median(data), color='r', linestyle='dashed', linewidth=1, label="Median {:.2f}".format(np.median(data)))
ax.axvline(data.std(), color='b', linestyle='dashed', linewidth=1, label="Std {:.2f}".format(data.std()))
ax.legend(loc=0)

plt.tight_layout()
plt.show()