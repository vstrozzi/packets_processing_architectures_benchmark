#include "../helpers/helpers.c"
#include <string.h> // memcpy

/* Reduction of N messages carrying M data items of type T in
   an array of size M*T.
   Data is received through simple IPV4 packets (port 7999 on ip 10.0.3.0) 
   and the reduction operation used here is a sum operator */

#define N TOT_SIZE_B
#define M DATA_NR

// Define BTF-map array (number of entries = number of cores)
struct {
        __uint(type, BPF_MAP_TYPE_ARRAY);
        __type(key, __u32);
        __type(value, T);
        __uint(max_entries, M);
} rxred SEC(".maps");

// Number of received pkts
struct {
        __uint(type, BPF_MAP_TYPE_ARRAY);
        __type(key, __u32);
        __type(value, T);
        __uint(max_entries, 1);
} rxpkt SEC(".maps");


// Macro to put compiled object in specific section of ELF
SEC("reduce_sec")
int reduce(struct xdp_md *ctx){
	// Get time of pkt_receival (from boot time and including time of system suspended)
	long long time = bpf_ktime_get_ns();
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
			// Check  if it is an UDP pkt
			if(ip->protocol == IPPROTO_UDP){
				struct udphdr *udp = (void *)ip + sizeof(*ip);
				// Verify udp component of the pkt contains enough data
				if((void *)udp + sizeof(*udp) <= pkt_end){
					// Get payload size (bytes)
					unsigned int payload_size = ntohs(udp->len) - sizeof(*udp);
					// Check whether we got the requested size
					if (payload_size != TOT_SIZE_B){
						return XDP_PASS;
					}
					// Point to start of payload and check boundaries
					int32_t *payload = (char *)udp + sizeof(*udp);
					if ((int32_t *) payload + payload_size/sizeof(T) > pkt_end){
						return XDP_PASS;
					}
					// Write up sum of values per core
                    __u32 key;
                    int32_t *map_val;
					for (int i = 0; i < payload_size/sizeof(T); i++){
                        // Get value of our per cpu reduction
                        key = i;
                        // Lookup for element and increment values with reduce operator sum
                        map_val = bpf_map_lookup_elem(&rxred, &key);
                        if(map_val){
							__sync_fetch_and_add(map_val, payload[i]);
                        }
						
					}

					// Send notification if we finished
					key = 0;
					int32_t *nr_pkt = bpf_map_lookup_elem(&rxpkt, &key);
					// Chec not 0 pointer for verifier
					if(nr_pkt){
						__sync_fetch_and_add(nr_pkt, 1);
						// Send back element if we have received correct number pkts
						if(*nr_pkt % (MiB/TOT_SIZE_B) == 0){
							unsigned short mem_udp = udp->dest;
							udp->dest = udp->source;
							udp->source = mem_udp;
						
							unsigned int mem_ip = ip->daddr;
							ip->daddr = ip->saddr;
							ip->saddr = mem_ip;
	
							unsigned char mem_eth[6];
							// Inlined clang memcpy
							memcpy(mem_eth, eth->h_dest, sizeof(char)*6);
							memcpy(eth->h_dest, eth->h_source, sizeof(char)*6);
							memcpy(eth->h_source, mem_eth, sizeof(char)*6);
							return XDP_TX;
						}
					}

					// Get time of pkt_processing (from boot time and including time of system suspended)
					if ((void *) payload + 2*sizeof(long long)> pkt_end){
						return XDP_PASS;
					}
					// Write in packet the time of receival and time after processing
					memcpy(payload, &time, sizeof(long long));
					time = bpf_ktime_get_ns();
					memcpy(((long long *) payload) + 1, &time, sizeof(long long));
					return XDP_PASS;
				}
			}
		
		}
	}
	// Pass pkt to network stack
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
