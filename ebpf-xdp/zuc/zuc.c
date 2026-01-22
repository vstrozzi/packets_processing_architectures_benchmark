#include "../helpers/helpers.c"
#include "zuc.h"
#include <string.h> // memcpy

/* Zuc implementation. A client can request to either encrypt or decrypt a msg.
   The header is as follow:
		- u128 Conf_Key (16 bytes)

	The rest of the pkt is to encrypt/decrypt. It generates a keystream of size 128 bits
*/

#define HEADER_SIZE_B sizeof(uint64_t)*2
#define PAYLOAD_SIZE_B TOT_SIZE_B - sizeof(struct ethhdr) - sizeof(struct iphdr) - sizeof(struct udphdr)

// Macro to put compiled object in specific section of ELF
SEC("zuc_sec")
int zuc(struct xdp_md *ctx){
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

					// Get init for cypher
					uint64_t c_key[2];
					c_key[0] = ((uint64_t *) payload)[0];
					c_key[1] = ((uint64_t *) payload)[1];
					
					payload = ((char *) payload) + HEADER_SIZE_B;
					uint32_t msg_len = payload_size - HEADER_SIZE_B;
					uint32_t out[PAYLOAD_SIZE_B];

					// Flag is ignored since ENC/DEC are the same ops 128-EEA3
					// Encrypt and Decrypt
					EEA3((uint8_t *) &c_key, msg_len, payload, out);
					
					
					// Copy content to the pkt
					payload = ((char *) payload) - HEADER_SIZE_B;
					for (int i = HEADER_SIZE_B; i < payload_size/DATA_SIZE_B; i++){
						payload[i] = out[i];
					}

					// Need to recompute checksum
					udp->check = 0;
					udp->check = caludpcsum(ip, udp, pkt_end);



					// Get time of pkt_processing (from boot time and including time of system suspended)
					if ((void *) payload + 2*sizeof(long long)> pkt_end){
						return XDP_PASS;
					}
					// Write in packet the time of receival and time after processing
					memcpy(payload, &time, sizeof(long long));
					time = bpf_ktime_get_boot_ns();
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
