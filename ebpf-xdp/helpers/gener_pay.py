# Write your parameters
MiB = 1048576
size_B  = MiB
data_size_B = 4
number_write = size_B//data_size_B
fname = "To_test.txt"


f = open(fname, "wb")
for i in range(number_write):
    # IMPORTANT: it is in low endian b'\x01\x00\x00\x00' = 1
    f.write(b'\x01\x00\x00\x00');

f.close()
