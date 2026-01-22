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
    prog = xdp_program__open_file("copy_from_host_no_t.o", "copy_from_host_sec", NULL);
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
    
    // Initialize data of map with whathever
    int32_t value[1];
    value[0] = 1;
    int key = 0;
    assert(bpf_map_update_elem(map_fd, &key, value, BPF_EXIST) == 0);
    for (int i = 0; i < DATA_NR; i++){
            key = i;
            assert(bpf_map_lookup_elem(map_fd, &key, value) == 0);
    }

    return 0;
}
