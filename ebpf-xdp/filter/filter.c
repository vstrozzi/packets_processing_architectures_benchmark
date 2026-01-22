#include "../helpers/helpers.c"
#include <string.h> // memcpy

/* Create filtering of data of simple IPV4 packets (port 7999 on ip 10.0.3.0) using 512 msgs
   on a 2^16 entries hash, where if a match is found on the IPv4 the UDP dest port 
   is overwritten and the content of the pkt is written to host memory (1024 bytest) */

#define HASH_ENTRIES 65536

// Define BTF-map array for written pkt content
struct {
        __uint(type, BPF_MAP_TYPE_ARRAY);
        __type(key, __u32);
        __type(value, int32_t);
        __uint(max_entries, TOT_SIZE_B);
} rxfilter SEC(".maps");

// Define Hash-map for matching IPv4
struct {
        __uint(type, BPF_MAP_TYPE_HASH);
        __type(key, __u32);
        __type(value, __u16);
        __uint(max_entries, HASH_ENTRIES);
} hmfilter SEC(".maps");

// Number of received pkts
struct {
        __uint(type, BPF_MAP_TYPE_ARRAY);
        __type(key, __u32);
        __type(value, T);
        __uint(max_entries, 1);
} rxpkt SEC(".maps");


// Define BTF-hash to match ips
// Macro to put compiled object in specific section of ELF
SEC("filter_sec")
int filter(struct xdp_md *ctx){
	// Record time of packet arrival
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
					if ((int32_t *) payload + payload_size/DATA_SIZE_B > pkt_end){
						return XDP_PASS;
					}
					// Check if IPv4 is in hash
					__u32 key = ntohs(ip->daddr);
					__u16 *map_val = bpf_map_lookup_elem(&hmfilter, &key);
					// Match found
					if(map_val != NULL){
					
                    // Copy received data to host
					for (int i = 0; i < payload_size/DATA_SIZE_B; i++){
                        key = i;
                        map_val = bpf_map_lookup_elem(&rxfilter, &key);
						// IMPORTANT: the verifier needs this check written like this to work
                        if(map_val != NULL){
							bpf_map_update_elem(&hmfilter, &key, &payload[i], BPF_ANY);
						}
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
					time = bpf_ktime_get_boot_ns();
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
