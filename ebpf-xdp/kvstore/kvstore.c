#include "../helpers/helpers.c"
#include <string.h> // memcpy

/* Kvstore on a cache with associativity ASSOCIATIVITY and
   a total Number of entries of NR_ENTRIES. The key is saved
   as a 32-bit integer in the first position of the payload.
   The set is determined by the key mod SETS_NR.
   The eviction policy is for now LRU to make use of hashes (ATOMIC)
   built in policies.
   
   
   FOR NOW: Every message is either read or write. 
   READ: read (32 bit) flag, key (32 bit), content (to write ret value)
   WRITE: write (32 bit) flag, key (32 bit), content (int32_bit)*/

#define SETS_NR 4
#define SETS_BLOCKS 500
#define CONTENT_TYPE T
#define DATA_READ_B TOT_SIZE_B // Now fixed to this for deb purposes: when fixed set it to data msg payload size
#define DATA_WRITE_B TOT_SIZE_B  // Now fixed to this for deb purposes: when fixed set it to data msg payload size
#define BUCK(v) v % SETS_NR

// Define BTF Maps (Same as SETS_NR)
struct {
        __uint(type, BPF_MAP_TYPE_LRU_HASH);
        __type(key, __u32);
        __type(value, CONTENT_TYPE);
        __uint(max_entries, SETS_BLOCKS);
} rxblk1 SEC(".maps");

struct {
        __uint(type, BPF_MAP_TYPE_LRU_HASH);
        __type(key, __u32);
        __type(value, CONTENT_TYPE);
        __uint(max_entries, SETS_BLOCKS);
} rxblk2 SEC(".maps");

struct {
        __uint(type, BPF_MAP_TYPE_LRU_HASH);
        __type(key, __u32);
        __type(value, CONTENT_TYPE);
        __uint(max_entries, SETS_BLOCKS);
} rxblk3 SEC(".maps");

struct {
        __uint(type, BPF_MAP_TYPE_LRU_HASH);
        __type(key, __u32);
        __type(value, CONTENT_TYPE);
        __uint(max_entries, SETS_BLOCKS);
} rxblk4 SEC(".maps");

// Function to get set based on value
void *get_set(int v){
	switch(v){
		case 1:
		return &rxblk1;
		case 2:
		return &rxblk2;
		case 3:
		return &rxblk3;
		case 4:
		return &rxblk4;
		// Impossible to get here
		default:
		return &rxblk1;
	}
}

// Macro to put compiled object in specific section of ELF
SEC("kvstore_sec")
int kvstore(struct xdp_md *ctx){
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
					if (payload_size != DATA_READ_B || payload_size != DATA_WRITE_B){
						return XDP_PASS;
					}

					// Point to start of payload and check boundaries
					int32_t *payload = (char *)udp + sizeof(*udp);
					if ((int32_t *) payload + payload_size/DATA_SIZE_B > pkt_end){
						return XDP_PASS;
					}

					int32_t flag = payload[0];
					// Write
					if(flag){
						// Add check for verifier
						if(payload_size != DATA_WRITE_B){
							return XDP_PASS;
						}
						// Get map pointed based on key
						__u32 key = payload[1];
						CONTENT_TYPE val = payload[2];
						void *map = get_set(BUCK(key));
						// Update hash entry
						bpf_map_update_elem(map, &key, &val, BPF_ANY);
					}
					// Read
					else{
						// Add check for verifier
						if(payload_size != DATA_READ_B){
							return XDP_PASS;
						}
						// Get map pointer based on key
						__u32 key = payload[1];
						void *map = get_set(BUCK(key));
						// Update hash entry
						CONTENT_TYPE *cont = bpf_map_lookup_elem(map, &key);
						// No content found
						if(cont != NULL){
							// Write content to packet after finding it
							payload[2] = *cont;
							// Need to recompute checksum
							udp->check = 0;
							udp->check = caludpcsum(ip, udp, pkt_end);
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

					// Now switch packets dest and source to forward it back
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


					// Clear and recompute checksum
					udp->check = 0;
					udp->check = caludpcsum(ip, udp, pkt_end);
					// Send packet_to socket with infos of time
					return XDP_TX;
				}
			}
		
		}
	}
	// Pass pkt to network stack
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
