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

// Global vars
static int ifindex;
struct xdp_program *prog = NULL;
int poll_time = 2;
int pid;

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
    prog = xdp_program__open_file("fw_back_ov.o", "fw_back_ov_sec", NULL);
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
    printf("Loading!\n");

    return 0;
}