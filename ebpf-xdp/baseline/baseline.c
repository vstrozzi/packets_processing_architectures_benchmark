#include "../helpers/helpers.c"
#include <string.h> // memcpy

/* Simple baseline which measures time of reception of a pkt and
   forward it to an open socket in the usr space 
   (IMPORTANT: the packet must have length of at least 8 to be overwriten with time,
    since XDP does not allow to extend the packet )
*/

// Macro to put compiled object in specific section of ELF
SEC("baseline_sec")
int baseline(struct xdp_md *ctx){
	// Get time of pkt_receival (from boot time and including time of system suspended)
	long long time = bpf_ktime_get_ns();
	// Get data
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
					// Point to start of payload and check boundaries are large 
					// enough to write time
					char *payload = (char *)udp + sizeof(*udp);
					if ((void *) payload + 2*sizeof(long long)> pkt_end){
						return XDP_PASS;
					}
					
					// Write in packet the time: arrival and processing (use built in llvm funcs)
					memcpy(payload, &time, sizeof(long long));
					time = bpf_ktime_get_boot_ns();
					memcpy(((long long *) payload) + 1, &time, sizeof(long long));
					
					// Clear and recompute checksum
					udp->check = 0;
					udp->check = caludpcsum(ip, udp, pkt_end);
					// Forward modified packet to user space
					return XDP_PASS;
				}
			}
		
		}
	}
	// Pass pkt to network stack
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
