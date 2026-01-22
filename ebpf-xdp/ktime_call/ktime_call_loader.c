#include <unistd.h>
#include <stdlib.h>
#include <time.h>  // Get time functions
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

#include "../helpers/helpers.h"
 
#define BUFLEN 2*sizeof(clock_t)
#define NS_PER_SEC 1000000000

// Global vars
static int ifindex;
struct xdp_program *prog = NULL;
int poll_time = 2;
int pid;

/* This function will remove XDP from the link when the program exits. */
static void int_exit(int sig){
    xdp_program__close(prog);
    printf("Unloading xdp and closing port\n");
    close(pid);
    exit(0);
}

/* This function will print the time at which a packet is received at the XDP interface
 */
static void poll_stats(){
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
	// Append benchmark to file stats
    const char fname[] = "ktime_call_ben.txt";
    FILE *fptr = fopen(fname, "a");
    if (fptr == NULL){
        printf("FILE not found\n");
        exit(1);
    }

	// Keep listening for data
	while(1){
		//printf("Waiting for data...");
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
            //printf("Diference of time in ns: %llu ns \n", usr_time_ns - *((unsigned long long*)buf));
            fprintf(fptr, "%llu\n", *((unsigned long long*)buf + 1) - *((unsigned long long*)buf));

        }
	}
    fclose(fptr);
	close(pid);
	return;
    
}

// Simply run the program with the name of the interface to attach the xdp to
int main(int argc, char *argv[]){
    int prog_fd, map_fd, ret;
    struct bpf_object *bpf_obj;

    if (argc != 2){
        printf("Need one argument: name interface");
        return 1;
    }

    // Get index of interface from name
    ifindex = if_nametoindex(argv[1]);
    if (!ifindex){
        printf("Could not find interface index (ifindex) from name\n");
        return 1;
    }

    // Load XDP object using libxdp
    prog = xdp_program__open_file("ktime_call.o", "ktime_call_sec", NULL);
    if (!prog) {
        printf("Could not load xdp prog \n");
        return 1;
    }

    // Attach XDP program to interface using skb mode
    // Please set ulimit if you got an -EPERM error.
    ret = xdp_program__attach(prog, ifindex, XDP_MODE_SKB, 0);
    if (ret){
        printf("Error, could not load xdp fd on %d\n", ifindex);
        return ret;
    }
    
    // Set signals to remove from interface when killed 
    signal(SIGINT, int_exit);
    signal(SIGTERM, int_exit);
    printf("Loading!\n");
    // Poll stats every
    poll_stats();

    return 0;
}