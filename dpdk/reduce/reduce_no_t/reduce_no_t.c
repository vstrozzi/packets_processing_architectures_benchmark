/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include <stdint.h>
#include <inttypes.h>
#include <signal.h>

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

#include "../../helpers/helpers.h"

/* Reduction of N messages carrying M data items of type T in
 * an array of size M*T.
 * Reduction operation used here is a + operator
 */

/* Parameters for reduce */
#define N TOT_SIZE_B
#define M DATA_NR

/* Global variables */
uint16_t core_que_nr;									/* Core number */
const char F_WRITE[] =
		"reduce_ben_proc.txt";
FILE *wfptr;
rte_rwlock_t *rwl;

RTE_DEFINE_PER_LCORE(uint64_t, cycles);    				/* Save cycles per each packet received in the burst */
volatile bool force_quit;	
T_rte *pkt_rec;							/* Number of received pkts */


/* Signal handler to stop lcores */
static void signal_handler(int signum){
	if (signum == SIGINT || signum == SIGTERM) {
		printf("\n\nSignal %d received, preparing to exit...\n",
				signum);
		force_quit = true;
	}
}

/* Function to initialize the port */
static inline int port_init(uint16_t port, struct rte_mempool *mbuf_pool){
	//if(port!=2)
	//	return 0;
	/* Copy predefined port conf with RSS */
	struct rte_eth_conf local_port_conf = port_conf;
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
	/* Check if offload of is possible */
	if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
		local_port_conf.txmode.offloads |=
			RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

	/* Set rss offload type */
	local_port_conf.rx_adv_conf.rss_conf.rss_hf &=
			dev_info.flow_type_rss_offloads;
	
	/* Check if max rx number is 1 and set appropriate flag*/
	if (dev_info.max_rx_queues == 1)
			local_port_conf.rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;

	/* HW has personal RSS hash function, use this */
	if (local_port_conf.rx_adv_conf.rss_conf.rss_hf !=
			local_port_conf.rx_adv_conf.rss_conf.rss_hf) {
		printf("Port %u modified RSS hash function based on hardware support,"
			"requested:%#"PRIx64" configured:%#"PRIx64"\n",
			port,
			port_conf.rx_adv_conf.rss_conf.rss_hf,
			local_port_conf.rx_adv_conf.rss_conf.rss_hf);
	}
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
	
	return 0;
}


/* Function run on lcores to poll on packets on a queue */
static int lcore_main(void **args){
	/* Get the args */
	rwl = (rte_rwlock_t *) args[0];
	T_rte **red_val = (T_rte **) args[1];

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

	/* Start reduce on every pkt */
	while(!force_quit){
		/* Reduce a received packet */
		RTE_ETH_FOREACH_DEV(port) {
			//if(port!=2)
			//	continue;
			/* Get burst (BURST_SIZE number) of RX packetsfrom port and queue id 0,
			 * store them in bufs and return
			 */
			struct rte_mbuf *bufs[BURST_SIZE];
			const uint16_t nb_rx = rte_eth_rx_burst(port, core_que_id,
					bufs, BURST_SIZE);
			/* In the case it is unlikely to have nb_rx == 0, continue */
			if (unlikely(nb_rx == 0))
				continue;
				
			/* Number meaningful pkts */
			uint16_t nb_mng_rx = 0;
			/* Start reduce on every packet */
			for(int i = 0; i < nb_rx; i++){
				struct rte_mbuf *pkt = bufs[i];
				/* Get Ether payload */
				struct rte_ether_hdr *eth_h;				
				eth_h = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
				/* Check if the received packet is of type IPv4. If not stop */
				if (rte_cpu_to_be_16(eth_h->ether_type) != RTE_ETHER_TYPE_IPV4){
					continue;
				}
				/* Get IP */
				struct rte_ipv4_hdr *ip_h = (struct rte_ipv4_hdr *) ((char *)eth_h
											 + sizeof(struct rte_ether_hdr));
				/* Get UDP */
				struct rte_udp_hdr *udp_h = (struct rte_udp_hdr *) ((char *)ip_h
											 + sizeof(struct rte_ipv4_hdr));
				uint16_t payload_size = ntohs(udp_h->dgram_len) - sizeof(struct rte_udp_hdr);
				/* Check whether payload has good length */
				if(payload_size != TOT_SIZE_B)
					continue;
				/* Get payload */
				T *payload = (T *) ((char *)udp_h + sizeof(struct rte_udp_hdr));
				/* Red up all int32 of the payload */
				for (int i = 0; i < payload_size/sizeof(T); i++){
					/* Lookup for element to reduce on increment values with reduce operator sum */
					T_rte *val = red_val[i];
					rte_atomic32_add(val, payload[i]);
				}
				/* Increment number meaningful pkts */
				nb_mng_rx += 1;
			}

			rte_atomic32_add(pkt_rec, nb_mng_rx);
			
			/* Send back if we finished aggregate */
                        if(rte_atomic32_read(pkt_rec) % (MiB/TOT_SIZE_B) == 0){

                                struct rte_mbuf *pkt = bufs[0];

                                /* Get Ether payload */
                                struct rte_ether_hdr *eth_h;
                                eth_h = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);

                                /* Get IP */
                                struct rte_ipv4_hdr *ip_h = (struct rte_ipv4_hdr *) ((char *)eth_h + sizeof(struct rte_ether_hdr));
                                /* Get UDP */
                                struct rte_udp_hdr *udp_h = (struct rte_udp_hdr *) ((char *)ip_h + sizeof(struct rte_ipv4_hdr));

                                /* Get payload size */
                                uint16_t payload_size = ntohs(udp_h->dgram_len) - sizeof(struct rte_udp_hdr);

                                //printf("The pkt has eth %d, Ip %d, UDP %d", eth_h->dst_addr, ip_h->dst_addr, udp_h->dst_port);        
                                /* Swap Ether */
                                struct rte_ether_addr eth_mem = eth_h->dst_addr;
                                eth_h->dst_addr = eth_h->src_addr;
                                eth_h->src_addr = eth_mem;

                                /* Swap IP */
                                rte_be32_t ip_mem = ip_h->dst_addr;
                                ip_h->dst_addr = ip_h->src_addr;
                                ip_h->src_addr = ip_mem;

                                /* Swap UDP */
                                rte_be16_t udp_mem = udp_h->dst_port;
                                udp_h->dst_port = udp_h->src_port;
                                udp_h->src_port = udp_mem;

                                rte_eth_tx_burst(port, core_que_id, bufs, 1);


                        }

			/* Free all packets */
			uint16_t buf;
			for (buf = 0; buf < nb_rx; buf++){
				if(bufs == NULL || bufs[buf] == NULL)
					continue;
				rte_pktmbuf_free(bufs[buf]);
			}		
		}
	}

	return 0;
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

	/* Set up signals */
	force_quit = false;
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* Check there is only one port */
	nb_ports = rte_eth_dev_count_avail();
	//if (nb_ports != 1){
	//	rte_exit(EXIT_FAILURE, "Need only one port to ping-pong\n");
	//}
	/* Get number of core for optimal usage of queue */
	core_que_nr = rte_lcore_count();
	printf("we have %d cores\n", core_que_nr);
	/* Creates a new mempool in memory to hold the mbufs. 1 per rx queue
	 * Allocates mempool to hold the mbuf per port 
	 */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL_PINGPONG", NB_MBUF(nb_ports, core_que_nr),
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
	
	/* Initializing all ports using above defined function and our mbuf_pool */
	RTE_ETH_FOREACH_DEV(portid)
		if (port_init(portid, mbuf_pool) != 0)
			rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
					portid);

	/* Pkt counter */
	pkt_rec = rte_malloc(NULL, sizeof(T_rte), 0);
	rte_atomic32_init(pkt_rec);
	/* Atomic counter to place values */
	T_rte *red_val[M];
	for(int i = 0; i < M; i++){
		red_val[i] = rte_malloc(NULL, sizeof(T_rte), 0);
		rte_atomic32_init(red_val[i]);
		rte_atomic32_set(red_val[i], 0);	
	}
	
	/* Init the RW-lock to be able to write to the file (need to be put in heap)*/
	rte_rwlock_t *rwl;		
	rwl = rte_malloc(NULL, sizeof(rte_rwlock_t), 0);
	rte_rwlock_init(rwl);
	printf("Launching all the lcores\n");

	/* Init arguments of lcore_main */
	void *args[2] = {rwl, red_val};
	
	/* Launches the function on each lcore (main aswell)*/
	rte_eal_mp_remote_launch((lcore_function_t *) lcore_main, args, CALL_MAIN);

	/* Wait all lcore */
	rte_eal_mp_wait_lcore();
	/* Clean up the EAL */
	rte_eal_cleanup();

	printf("Bye bye...\n");

	return 0;
}
