import chunk
import matplotlib.pyplot as plt
import numpy as np
import subprocess as sp
import sys
import os
import signal
import time
from pathlib import Path
import shutil

# ARGUMENTS:
#           - 0: vnet/name of interface
#           - 1: coll (collect data)

# Not example directories
NOT_EX = ["helpers", "plot", "benchmarks_utilities"]
# Sizes to send
SIZES = [1024] 
FILE_SIZE = 1048576 # 1MiB

# Get arguments
argv = sys.argv[1:]

# Get parent path
path = Path.cwd().resolve()
path_par = Path.cwd().parent.resolve()

# Examples directories
examples_dir = [d for d in path_par.iterdir() if
            d.is_dir()
            and d.name not in NOT_EX
            and not str(d.name).startswith(".")
            and not str(d.name).startswith("loadbal")
            and not str(d.name).startswith("kvstor")
            and not str(d.name).startswith("ping")]

# Helpers dir
helper_dir = [d for d in path_par.iterdir() if
            d.name == "helpers"][0]

# Helpers' header file to edit
helper_head = [h for h in helper_dir.iterdir() if
               str(h.name).startswith("helpers.h")][0]

 # Set up vnet
if argv[0] == "vnet":
    print("Setting up vnet")
    # Wait some time to set up everything correctly
    time.sleep(2)
    run_cmd = ["sudo", "./create_vnet.sh"]
    proc = sp.Popen(run_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=helper_dir)
    proc.communicate()
    time.sleep(2)
    print("Vnet set up")

if argv[1] == "coll":
    # Enlarge buffers 
    print("Enlarging buffers")
    run_cmd = ["sudo", "./ext_buff_size.sh"]
    proc = sp.Popen(run_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=helper_dir)
    proc.communicate()
    time.sleep(2)
    print("Buffers enlarged")
    print()

    # Run test for different sizes
    for size in SIZES:
        # Edit file helpers.h in helpers with the new size
        print("We are using size %d" % size)
        print()
        file = helper_head.open("r")
        list_of_lines = file.readlines()
        count = 0
        for line in list_of_lines:
            if str(line).startswith("#define TOT_SIZE_B "):
                list_of_lines[count] = "#define TOT_SIZE_B " + str(size) + "\n"
            count += 1

        if count == len(list_of_lines) - 1:
            print("Cannot find TOT_SIZE_B in helpers.h")
            exit(0)
        
        # Write back
        file = helper_head.open("w")
        file.writelines(list_of_lines)
        file.close()

        # Run example for each example_dir
        for dir in examples_dir:
            print("Running " + str(dir.name))
            # Compile the program
            make_cmd = ["sudo", "./compile.sh"]
            make_proc = sp.Popen(make_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=dir)
            make_proc.communicate()

            # Check if vnet is required
            run_cmd = []
            if argv[0] == "vnet":
                print("Loading on veth1")
                run_cmd = ["sudo", "./" + dir.name,  "veth1"]
            else:
                run_cmd = ["sudo", "./" + dir.name,  str(argv[0])]
            
            # Run process
            run_proc = sp.Popen(run_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=dir)
            
            time.sleep(3)
            
            print("Start sending")
            
            # Run test
            test_cmd = ["python3", "test.py", str(size)]
            test_proc = sp.Popen(test_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=helper_dir)
            # Wait completion
            test_proc.communicate()
            time.sleep(5)
            
            # Kill original running process (SIGTERM)
            os.system("sudo pkill -9 -P " + str(run_proc.pid))
            run_proc.communicate()

            # Clean directory
            clean_command = ["sudo", "rm", dir.name +".o", dir.name + "_loader"]
            clean_proc = sp.Popen(clean_command, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=dir)
            clean_proc.communicate()

            # Unload program from interface
            run_cmd = []
            if argv[0] == "vnet":
                run_cmd = ["sudo", "xdp-loader",  "unload", "-a", "veth1"]
            else:
                run_cmd = ["sudo", "xdp-loader",  "unload", "-a", str(argv[0])]
            print("Finish with " + str(dir.name))
            print()

    time.sleep(5)

    print("Copying benchmark file to plot/")
    # Copy output file in our dir
    for dir in examples_dir:
        file_ben = [f for f in dir.iterdir() if str(f.name).endswith(".txt")][0]
        shutil.move(str(file_ben), str(path))

    print("Copied all files")

    # Restore buffers 
    print("Restoring buffers")
    run_cmd = ["sudo", "./def_buff_size.sh"]
    proc = sp.Popen(run_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=helper_dir)
    proc.communicate()
    time.sleep(1)
    print("Buffers restored")


 # Destroy vnet
if argv[0] == "vnet":
    print("Destroying vnet")
    # Wait some time to set up everything correctly
    time.sleep(2)
    run_cmd = ["sudo", "./delete_vnet.sh"]
    proc = sp.Popen(run_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=helper_dir)
    proc.communicate()
    time.sleep(2)
    print("Vnet deleted")

# Start plotting
print("Start plotting")

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
    data_loc = []
    file = open(str(lab), "r")
    # Read benchmark for given size
    for size in SIZES:
        chunks_nr = FILE_SIZE//size
        # Compute Throughput
        time = 0
        for i in range(0, chunks_nr):
            line = file.readline()
            time += int(line)
        # Compute throughput in Gbit/s (substract overhead call time)
        data_loc.append(round((FILE_SIZE*8)/(time), 2))
        
    data.append(data_loc)
    file.close()

# Create data array
data_real = []
for i in range(0, len(SIZES)):
    data_loc = []
    for j in range (0, len(labels)):
        data_loc.append(data[j][i])
    data_real.append(data_loc)


x = np.arange(len(labels))  # the label locations
width = 0.30  # the width of the bars

fig, ax = plt.subplots()

# Creates rects
count = 0
step = width
base = x - width
rects = []
for size in SIZES:
    rects.append(ax.bar(base + step*count, data_real[count], width, label=str(size) + " B"))
    count += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Gbit/s')
ax.set_title('Application')
ax.set_xticks(x, labels)
ax.legend()

for rect in rects:
    ax.bar_label(rect, padding=3)

fig.tight_layout()

plt.show()

