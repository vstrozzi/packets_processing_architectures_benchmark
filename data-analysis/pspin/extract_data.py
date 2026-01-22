# HardCoded extractor of data from PsPIN's docker simulation

# filter useless lines
def check_info_feed(line):
    if "[src/NICInbound.hpp:473]: INFO FEEDBACK" in line:
          return True  
    return False

# map out useless
def map_out(line):
    line = line[line.find('K')+1:]
    return line[11:].split(" ")[1]

file1 = open("benchmark.txt", "r")
file2 = open("edit_benchmark.txt", "w")

for line in file1:
    if(check_info_feed(line)):
        print(line)

        # Extract elements from the numbers list for which check_even() returns True
        file2.write(str(map_out(line)) + "\n")

file1.close()
file2.close()
