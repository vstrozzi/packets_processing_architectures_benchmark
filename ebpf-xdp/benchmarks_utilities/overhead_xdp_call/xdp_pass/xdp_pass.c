#include "../../../helpers/helpers.c"

/* Simple pass to usr space */

// Macro to put compiled object in specific section of ELF
SEC("xdp_pass_sec")
int xdp_pass(struct xdp_md *ctx){
	// Pass pkt to network stack
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
