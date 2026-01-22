import socket
import sys
import time

# Infos on where to send (UDP) (like this we can try 4 cores)
IP = "10.0.3.0"
PORTS = [7999, 8000, 8001, 8002]
# Size of payload chunks (BYTE) --> Needs to divide size of file
if len(sys.argv) != 2:
    print("Got no size as argument")
    exit(-1)

SIZE = int(sys.argv[1])

# Message in byte format
m = open("To_test.txt", 'rb')
# Size of file 
MiB = 1048576
size = MiB
# Number of chunks
chunks = size//SIZE

# Check correct size
if (MiB % SIZE != 0):
    print("Size and size file not divisor\n")
    exit(1)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Loop to send a message splitted in chuks
for i in range(0,chunks):
    # Read in bytes
    payload = m.read(SIZE)
    # Send message
    sock.sendto(payload, (IP, PORTS[0]))


    # Sleep some time to not overload the receiver

# Finish with sending chunks
print("Finish with sending chunks")
m.close()
