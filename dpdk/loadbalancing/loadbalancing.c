/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_common.h>
#include <rte_config.h>
#include <rte_errno.h>
#include <rte_ip.h>
#include <rte_malloc.h>

/* Simple Load Balacing (using whether algo we want) on the IP-Addres v4 src*/


/* Parameters for Port init */
#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

#define NS_PER_SEC 1000000000


/* Global variables */
uint16_t core_que_nr; 					/* Core number */
const char F_WRITE[] =
		"pingpong_ben_proc.txt";
FILE *wfptr;

uint64_t cycles[BURST_SIZE]; 			/* Save cycles per each packet received in the burst */

/* Simple hash algo */
int32_t sdbm(char *word, int32_t len){
	int32_t hashAddress = 0;
	for (int counter = 0; counter < len; counter++){
		hashAddress = word[counter] + (hashAddress << 6) + (hashAddress << 16) - hashAddress;
	}
	return hashAddress;
}


/* Rx Callback function to measure time of receival on Burst */
static uint16_t rx_latency(uint16_t port __rte_unused, uint16_t qidx __rte_unused,
	struct rte_mbuf **pkts, uint16_t nb_pkts,
	uint16_t max_pkts __rte_unused, void *_ __rte_unused){
	uint64_t now = rte_rdtsc_precise();
	/* Set latency for each pkt received in burst */
	for (int i = 0; i < nb_pkts; i++)
		cycles[i] = now;

	return nb_pkts;
}

/* TX Callback to measure time of sending on Burst */
static uint16_t calc_latency(uint16_t port, uint16_t qidx __rte_unused,
	struct rte_mbuf **pkts, uint16_t nb_pkts, void *_ __rte_unused){
	uint64_t latency = 0;
	uint64_t queue_ticks = 0;
	uint64_t now = rte_rdtsc_precise();
	uint64_t ticks;

	/* Get frequency lcore */
	uint64_t hz_lcore = rte_get_tsc_hz();
	latency = (now - cycles[0]);
	/* Processing */
	latency = latency*((__float80)NS_PER_S/((__float80)hz_lcore)); 

	printf("We are calc_lateny\n");
	printf("For port %d, on lcore %d, on queue %d, for a retrieved burst of size %d, having a frequency of %lu, we got a total latency of %lu\n",
		    port, rte_lcore_id(), qidx, nb_pkts, hz_lcore, latency);
	
	/* Write data for benchmark */
    if ((wfptr = fopen(F_WRITE, "a")) == NULL){
        printf("FILE not found\n");
        exit(1);
    }
    /* Write previously gathered data to file (average of it per packet) */
    for(int i = 0; i < nb_pkts; i++)
        fprintf(wfptr, "%lu\n", latency);
    fclose(wfptr);

	return nb_pkts;
}


/* Function to initialize the port */
static inline int port_init(uint16_t port, struct rte_mempool *mbuf_pool){
	/* Define struct for info of given port */
	struct rte_eth_conf port_conf;
	/* Number of rings */
	const uint16_t rx_rings = core_que_nr, tx_rings = core_que_nr;
	/* Define size of rings (receiving and sending) */
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;
	/* Check given port is valid */
	if (!rte_eth_dev_is_valid_port(port))
		return -1;
	/* Init port_conf to 0 */
	memset(&port_conf, 0, sizeof(struct rte_eth_conf));

	/* Retrieve infos from port in dev_info */
	retval = rte_eth_dev_info_get(port, &dev_info);
	if (retval != 0) {
		printf("Error during getting device (port %u) info: %s\n",
				port, strerror(-retval));
		return retval;
	}
	/* Check if offload is possible I think */
	if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
		port_conf.txmode.offloads |=
			RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

	/* Configure the Ethernet device using defined number of rx, tx rings
	 * and port_conf settings
	 */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	/* Adjust number of descriptors rx and tx per ring */
	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0)
		return retval;

	/* Allocate and set up rx_rings number of RX QUEUE per Ethernet port (1 here)
	 * in mbuf_pool
	 */
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
				rte_eth_dev_socket_id(port), NULL, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;
	/* Allocate and set up tx_rings number of TX queue per Ethernet port */
	for (q = 0; q < tx_rings; q++) {
		retval = rte_eth_tx_queue_setup(port, q, nb_txd,
				rte_eth_dev_socket_id(port), &txconf);
		if (retval < 0)
			return retval;
	}

	/* Start the Ethernet port */
	retval = rte_eth_dev_start(port);

	if (retval < 0)
		return retval;

	/* Print the port MAC address */
	struct rte_ether_addr addr;

	retval = rte_eth_macaddr_get(port, &addr);
	if (retval != 0)
		return retval;

	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port, RTE_ETHER_ADDR_BYTES(&addr));

	/* Enable RX in promiscuous mode for the Ethernet device. */
	retval = rte_eth_promiscuous_enable(port);
	/* End of setting RX port in promiscuous mode. */
	if (retval != 0)
		return retval;

	/* Add RX and TX functions */
	rte_eth_add_rx_callback(port, 0, rx_latency, NULL);
	rte_eth_add_tx_callback(port, 0, calc_latency, NULL);

	return 0;
}


static __rte_noreturn void lcore_main(){
	/* Set up var on which queue to read and write */
	uint16_t core_que_id = rte_lcore_id();
	/* Check that the port is on the same NUMA node as the polling thread for best performance. */
	uint16_t port;
	RTE_ETH_FOREACH_DEV(port)
		if (rte_eth_dev_socket_id(port) >= 0 &&
				rte_eth_dev_socket_id(port) !=
						(int)rte_socket_id())
			printf("WARNING, port %u is on remote NUMA node to "
					"polling thread.\n\tPerformance will "
					"not be optimal.\n", port);

	printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
			core_que_id);

	/* Main work of application driver */
	for (;;) {
		/* Ping pong a received packet */
		RTE_ETH_FOREACH_DEV(port) {
			/* Get burst (BURST_SIZE number) of RX packetsfrom port and queue id 0,
			 * store them in bufs and return
			 */
			struct rte_mbuf *bufs[BURST_SIZE];
			const uint16_t nb_rx = rte_eth_rx_burst(port, core_que_id,
					bufs, BURST_SIZE);
			/* In the case it is unlikely to have nb_rx == 0, continue */
			if (unlikely(nb_rx == 0))
				continue;
			/* Send back every packed received */
			uint16_t nb_tx = 0;
			for(int i = 0; i < nb_rx; i++){
				struct rte_mbuf *pkt = bufs[i];

				/* Get Ether payload */
				struct rte_ether_hdr *eth_h;				
				eth_h = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
				/* Check if the received packet is of type IPv4. If not stop */
				if (rte_cpu_to_be_16(eth_h->ether_type) != RTE_ETHER_TYPE_IPV4)
					continue;
				/* Get Ether */
				struct rte_ether_addr eth_mem = eth_h->dst_addr;

				/* Get IP */
				struct rte_ipv4_hdr *ip_h = (struct rte_ipv4_hdr *) ((char *)eth_h + sizeof(struct rte_ether_hdr));
				
				/* See if the packet is on correct queue for us */
				uint8_t load_b_queue = sdbm((char *) &(ip_h->src_addr), sizeof(ip_h->src_addr)) % core_que_nr;
				/* Correct queue, do some processing */
				if(load_b_queue == rte_que_id){
					/* Processing */
				}else{
					/* Send the packet to correct queue */
					nb_tx += rte_eth_tx_burst(port, , &pkt, 1);
				}
				
			}

			/* Free any unsent packets if the packets sent are less than the received one */
			if (unlikely(nb_tx < nb_rx)) {
				uint16_t buf;
				for (buf = nb_tx; buf < nb_rx; buf++)
					rte_pktmbuf_free(bufs[buf]);
			}
		}
	}
}

/* First function called. Call functions per core */
int main(int argc, char *argv[]){
	struct rte_mempool *mbuf_pool;
	unsigned nb_ports;
	uint16_t portid;
	/* Initializion the Environment Abstraction Layer (EAL) by passing the args */
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	argc -= ret;
	argv += ret;
	/* Check there is only one port */
	nb_ports = rte_eth_dev_count_avail();
	if (nb_ports != 1){
		rte_exit(EXIT_FAILURE, "Need only one port to ping-pong\n");
	}
	/* Get number of core for optimal usage of queue */
	core_que_nr = rte_lcore_count();
	printf("we have %d cores\n", core_que_nr);
	/* Creates a new mempool in memory to hold the mbufs. 1 per rx queue
	 * Allocates mempool to hold the mbuf per port 
	 */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL_PINGPONG", NUM_MBUFS * nb_ports * core_que_nr,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
	
	/* Initializing all ports using above defined function and our mbuf_pool */
	RTE_ETH_FOREACH_DEV(portid)
		if (port_init(portid, mbuf_pool) != 0)
			rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
					portid);

	/* Print number of lcore to use */
	if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	
	/* Launches the function on each lcore */
	/* unsigned lcore_id;
	RTE_LCORE_FOREACH_WORKER(lcore_id) {
		rte_eal_remote_launch(lcore_main, NULL, lcore_id);
	}
	*/
	/* Call lcore_main on the main core only. Called on single lcore */
	lcore_main();

	/* rte_eal_mp_wait_lcore(); */
	/* Clean up the EAL */
	rte_eal_cleanup();

	return 0;
}
