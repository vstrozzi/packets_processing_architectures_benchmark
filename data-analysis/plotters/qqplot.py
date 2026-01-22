import statsmodels.api as sm
import pylab
import numpy as np

file = open("dpdk" + "/" + "bl2_copy_to_host_ben_proc.txt", "r")
# Read benchmark for given size
data_loc = []
chunks_nr = 32768
# Compute Throughput
test = np.empty(chunks_nr)
for i in range(0, chunks_nr):
    test[i] =  int(file.readline())
    # Compute throughput in Gbit/s (substract overhead call time)

sm.qqplot(test, line='45')
pylab.show()