import numpy as np

# Gather data from file
data_fw = np.loadtxt("./fw_back/fw_back_ben.txt", dtype=int, delimiter="\n")
data_fw_ov = np.loadtxt("./fw_back_ov/fw_back_ov_ben.txt", dtype=int, delimiter="\n")

# Get median of overhead bpf_ktime_call
print(np.mean(data_fw_ov) - np.mean(data_fw))
