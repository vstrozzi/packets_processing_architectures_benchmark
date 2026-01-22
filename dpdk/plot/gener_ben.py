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
#           - 0: nr_cores
#           - 1: tap
#           - 2: coll (collect data)

# Not example directories
NOT_EX = ["helpers", "plot"]
# Sizes to send
SIZES = [1024, 512, 64] 
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
            and not str(d.name).startswith("loadbal")]

# Helpers dir
helper_dir = [d for d in path_par.iterdir() if
            d.name == "helpers"][0]

# Helpers' header file to edit
helper_head = [h for h in helper_dir.iterdir() if
               str(h.name).startswith("helpers.h")][0]

def collectData():
    # Run test for different sizes
    for size in SIZES:
        # Edit file in helper head with the new size
        print("We are using size %d" % size)
        print()
        file = helper_head.open("r")
        list_of_lines = file.readlines()
        if not list_of_lines[1].startswith("#define TOT_SIZE_B"):
            print(list_of_lines[1])
            print("The file of helpers.h probably got edited. Cannot find TOT_SIZE_B at line 1")
            exit(0)

        list_of_lines[1] = "#define TOT_SIZE_B " + str(size) + "\n"
        
        file = helper_head.open("w")
        file.writelines(list_of_lines)
        file.close()

        # Run example for each example_dir
        for dir in examples_dir:
            print("Running " + str(dir.name))
            # Run makefile
            make_cmd = ["make"]
            make_proc = sp.Popen(make_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=dir)
            make_proc.communicate()
            
            # Check if tap is required
            run_cmd = []
            if argv[1] == "tap":
                print("Using tap device ")
                run_cmd = ["sudo", "./build/" + dir.name,  "-l" + " 0-" +  str(int(argv[0]) - 1), "--vdev=net_tap0,iface=tap0"]
            else:
                run_cmd = ["sudo",  "./build/" + dir.name,  "-l" + " 0-" + str(int(argv[0]) - 1)]
            
            # Run process
            run_proc = sp.Popen(run_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=dir)
            
            # Run after tap if tap
            if argv[1] == "tap":
                # Wait some time to set up everything correctly
                time.sleep(2)
                run_cmd = ["sudo", "./after_tap.sh"]
                proc = sp.Popen(run_cmd, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=dir)
                stoud, stderr = proc.communicate()
                time.sleep(2)
            
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

            # Clean makefile
            make_command = ["make", "clean"]
            make_proc = sp.Popen(make_command, shell=False, stdout=sp.PIPE, stderr=sp.STDOUT, cwd=dir)
            make_proc.communicate()

            print("Finish with " + str(dir.name))
            print()

    time.sleep(5)

    print("Copying benchmark file to plot/")
    # Copy output file in our dir
    for dir in examples_dir:
        file_ben = [f for f in dir.iterdir() if str(f.name).endswith(".txt")][0]
        shutil.move(str(file_ben), str(path))

    print("Copied all files")


def plot():
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


def main():
    # Gather data for examples
    if argv[2] == "coll":
        collectData()
          
    # Start plotting
    print("Start plotting")
    
    plot()


if __name__ == "__main__":
    main()
