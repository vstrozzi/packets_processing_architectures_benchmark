#include "../helpers/helpers.c"
#include <string.h> // memcpy

/* Simple Load Balacing (using whether algo we want) on the IP-Addres v4 src*/

// Simple hash algo
int32_t sdbm(char *word, int32_t len){
	int32_t hashAddress = 0;
	for (int counter = 0; counter < len; counter++){
		hashAddress = word[counter] + (hashAddress << 6) + (hashAddress << 16) - hashAddress;
	}
	return hashAddress;
}

// Helpers
#define NR_RX 1
#define BUCK(v) (v % NR_RX)

// Macro to put compiled object in specific section of ELF
SEC("loadbalancing_sec")
int loadbalancing(struct xdp_md *ctx){
	// Get time of pkt_receival (from boot time and including time of system suspended)
	long long time = bpf_ktime_get_boot_ns();
	// Get pkt
	void *pkt_start = (void *) ctx->data;
    void *pkt_end = (void *) ctx->data_end;	
	// Check needed since we are bypassing kernel
	// Verify eth component of the pkt contains enough data
	struct ethhdr *eth = pkt_start;
	if((void *) eth + sizeof(*eth) <= pkt_end){
		struct iphdr *ip = pkt_start + sizeof(*eth);
		// Verify ip component of the pkt contains enough data
		if((void *)ip + sizeof(*ip) <= pkt_end){
			// Check if it is an UDP pkt
			if(ip->protocol == IPPROTO_UDP){
				struct udphdr *udp = (void *)ip + sizeof(*ip);
				// Verify udp component of the pkt contains enough data
				if((void *)udp + sizeof(*udp) <= pkt_end){
					// Point to start of payload and check boundaries
					int32_t *payload = (char *)udp + sizeof(*udp);
					// Get time of pkt_processing (from boot time and including time of system suspended)
					if ((void *) payload + 2*sizeof(long long)> pkt_end){
						return XDP_PASS;
					}
					// Write in packet the time of receival and time after processing
					memcpy(payload, &time, sizeof(long long));
					time = bpf_ktime_get_boot_ns();
					memcpy(((long long *) payload) + 1, &time, sizeof(long long));
					// Update rx queue using hash algo
					ctx->rx_queue_index = BUCK(sdbm(&(ip->saddr), sizeof(ip->saddr)));
					return XDP_PASS;
				}
			}
		
		}
	}
	// Pass pkt to network stack
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
