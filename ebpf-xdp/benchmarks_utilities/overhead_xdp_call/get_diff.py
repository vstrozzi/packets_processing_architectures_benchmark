import numpy as np

# Gather data from file
data_pass = np.loadtxt("./xdp_pass/xdp_pass_ben.txt", dtype=int, delimiter="\n")
data_no_pass = np.loadtxt("./xdp_no_pass/xdp_no_pass_ben.txt", dtype=int, delimiter="\n")

# Get median of overhead bpf_ktime_call
print(np.mean(data_pass) - np.mean(data_no_pass))
