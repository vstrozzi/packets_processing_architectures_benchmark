#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_link.h>
#include <signal.h>
#include <net/if.h>
#include <assert.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <xdp/libxdp.h>

#include <arpa/inet.h>
#include <sys/socket.h> // Socket library
#include <time.h>  // Get time functions

#include "../helpers/helpers.h" // For benchmarking purposes 

#define BUFLEN 2*sizeof(clock_t)
#define NS_PER_SEC 1000000000

#define DATA_TO_COPY_B TOT_SIZE_B


// Global vars
static int ifindex;
struct xdp_program *prog = NULL;
const char map_name[] = "rxcptoh";
int pid;

/* This function will remove XDP from the link when the program exits. */
static void int_exit(int sig)
{
    xdp_program__close(prog);
    printf("Unloading xdp\n");
    close(pid);
    exit(0);
}

/* This function will count the per-CPU number of packets and print out
 * the total number of dropped packets number and PPS (packets per second).
 */
static void poll_stats(int map_fd){
    struct sockaddr_in sock_to, sock_from;
    int sock_len = sizeof(sock_to);
    int pay_len;
	char buf[BUFLEN];
	
	// Create a UDP socket
	if ((pid=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        printf("Error creating socket\n");
		exit(1);
	}
	
	// Zero out the structure
	memset((char *) &sock_to, 0, sizeof(sock_to));
	
    // Set our server socket parameters
	sock_to.sin_family = AF_INET;
	sock_to.sin_port = htons(PORT);
	sock_to.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind socket to port
	if(bind(pid, (struct sockaddr*)&sock_to, sizeof(sock_to)) == -1){
		printf("Error binding to port\n");
		exit(1);
	}
	
    // Define stats to read data's map update
    // Get number of cpus
    int ncpus = libbpf_num_possible_cpus();
    if (ncpus < 0) {
        printf("Wrong number of cpus\n");
        return;
    }
    // Append benchmark to file stats
    const char fname[] = "copy_from_host_ben.txt";
    FILE *fptr = fopen(fname, "a");
    if (fptr == NULL){
        printf("FILE not found\n");
        exit(1);
    }

    // Initialize data of map with whathever
    int32_t value[1];
    value[0] = 1;
    int key = 0;
    assert(bpf_map_update_elem(map_fd, &key, value, BPF_EXIST) == 0);
    for (int i = 0; i < DATA_NR; i++){
            key = i;
            assert(bpf_map_lookup_elem(map_fd, &key, value) == 0);
    }

	// Keep listening for data
	while(1){
		//printf("Waiting for data...\n");
		fflush(stdout);

		// Try to receive some data (blocking)
		if((pay_len = recvfrom(pid, buf, BUFLEN, 0, (struct sockaddr *) &sock_from, &sock_len)) == -1){
			printf("Error from receiving\n");
		    exit(1);
		}
        // Save time of receival (important: use same clock_id as xdp kern)
        struct timespec rec;
        clock_gettime(CLOCK_MONOTONIC, &rec);
        unsigned long long usr_time_ns = (unsigned long long)rec.tv_nsec + ((unsigned long long)rec.tv_sec)*NS_PER_SEC;
 
		// Print details of the client/peer and the data received
		//printf("Received packet from %s:%d\n", inet_ntoa(sock_from.sin_addr), ntohs(sock_from.sin_port));
        // Read time from boot if possible (long long )
        if(pay_len >= BUFLEN){
            // Print time of receival in different formats
            //printf("Time of XDP_kern receival: %llu ns\n" , *((unsigned long long*)buf));
            //printf("Time of XDP_kern processing: %llu ns\n" , *((unsigned long long*)buf + 1));
            //printf("Tot time of XDP_kern processing: %llu ns \n", *((unsigned long long*)buf + 1) - *((unsigned long long*)buf));
            //printf("Time of user_space receival: %llu ns\n" , usr_time_ns);
            //printf("Difference of time in ns from receival: %llu ns \n", usr_time_ns - *((unsigned long long*)buf));
            fprintf(fptr, "%llu\n", *((unsigned long long*)buf + 1) - *((unsigned long long*)buf));

        }
	}
    fclose(fptr);
	close(pid);
}

// Simply run the program with the name of the interface to attach the xdp to
int main(int argc, char *argv[])
{
    int prog_fd, map_fd, ret;
    struct bpf_object *bpf_obj;

    if (argc != 2) {
        printf("Need one argument: name interface");
        return 1;
    }

    // Get index of interface from name
    ifindex = if_nametoindex(argv[1]);
    if (!ifindex) {
        printf("Could not find interface index (ifindex) from name\n");
        return 1;
    }

    // Load XDP object using libxdp
    prog = xdp_program__open_file("copy_from_host.o", "copy_from_host_sec", NULL);
    if (!prog) {
        printf("Could not load xdp prog \n");
        return 1;
    }

    // Attach XDP program to interface using skb mode
    // Please set ulimit if you got an -EPERM error.
    ret = xdp_program__attach(prog, ifindex, XDP_MODE_SKB, 0);
    if (ret) {
        printf("Error, could not load xdp fd on %d\n", ifindex);
        return ret;
    }

    // Get the map fd from the bpf object
    bpf_obj = xdp_program__bpf_obj(prog);
    map_fd = bpf_object__find_map_fd_by_name(bpf_obj, map_name);
    if (map_fd < 0) {
        printf("Could not get map fd from map name \n");
        return map_fd;
    }

    // Set signals to remove from interface when killed 
    signal(SIGINT, int_exit);
    signal(SIGTERM, int_exit);
    printf("Loading!\n");
    // Poll stats every
    poll_stats(map_fd);

    return 0;
}
