// Simple macros with our example port and other utilities
#define MAX_UDP_SIZE 1480
#define IPV4_ADDR(a, b, c, d)(((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff))
#define PORT 8000 
#define IP "10.0.1.0"


// Data values per test
#define TOT_SIZE_B 1024
#define T int32_t
#define DATA_SIZE_B sizeof(T)
#define DATA_NR (TOT_SIZE_B / DATA_SIZE_B)
#define MiB 1048576
#define NR_MSGS MiB/TOT_SIZE_Bs
#define NR_REP 1000
