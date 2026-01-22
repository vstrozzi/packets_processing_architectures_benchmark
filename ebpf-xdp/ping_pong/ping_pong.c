#include "../helpers/helpers.c"
#include <string.h> // memcpy

/* ping_pong for simple IPV4 packets */

// Macro to put compiled object in specific section of ELF
SEC("ping_pong_sec")
int ping_pong(struct xdp_md *ctx){
	// Get time of pkt_receival (from boot time and including time of system suspended)
	long long time = bpf_ktime_get_ns();
	void *pkt_start = (void *) ctx->data;
    void *pkt_end = (void *) ctx->data_end;
	// Check needed since we are bypassing kernel
	// Verify eth component of the pkt contains enough data
	struct ethhdr *eth = pkt_start;
	if((void *) eth + sizeof(*eth) <= pkt_end){
		struct iphdr *ip = pkt_start + sizeof(*eth);
		// Verify ip component of the pkt contains enough data
		if((void *)ip + sizeof(*ip) <= pkt_end){
			// Check it is an UDP pkt
			if(ip->protocol == IPPROTO_UDP){
				struct udphdr *udp = (void *)ip + sizeof(*ip);
				// Verify udp component of the pkt contains enough data
				if((void *)udp + sizeof(*udp) <= pkt_end){
					// Get payload
					int32_t *payload = (char *)udp + sizeof(*udp);
					// Swap dest and source (NO NEED to recompute checksum)					
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
					// Get time of pkt_processing (from boot time and including time of system suspended)
					if ((void *) payload + 2*sizeof(long long)> pkt_end){
						return XDP_PASS;
					}
					
					// Write in packet the time of receival and time after processing
					memcpy(payload, &time, sizeof(long long));
					time = bpf_ktime_get_ns();
					memcpy(((long long *) payload) + 1, &time, sizeof(long long));
					// Clear and recompute checksum
					udp->check = 0;
					udp->check = caludpcsum(ip, udp, pkt_end);
					// Redirect modified pkt to same interface
					return XDP_TX;
				}
			}
		
		}
	}
	// Pass pkt to network stack
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
