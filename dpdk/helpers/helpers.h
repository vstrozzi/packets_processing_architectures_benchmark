/* Parameters for test */
#define TOT_SIZE_B 1024
#define T int32_t
#define T_rte rte_atomic32_t
#define DATA_SIZE_B sizeof(T)
#define DATA_NR (TOT_SIZE_B / DATA_SIZE_B)
#define MiB 1048576
#define NR_MSGS MiB/TOT_SIZE_B

/* Parameters for Port init */
#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 64

#define NS_PER_SEC 1000000000

/* Macro to calculate needed space for memory pool */
#define NB_MBUF(nports, nb_lcores) RTE_MAX(	\
	(nports*core_que_nr*RX_RING_SIZE +		\
	nports*nb_lcores*BURST_SIZE +	\
	nports*core_que_nr*TX_RING_SIZE +		\
	nb_lcores*MBUF_CACHE_SIZE),		\
	(unsigned)NUM_MBUFS)

/* Define layout port-conf with RSS */
struct rte_eth_conf port_conf = {
	.rxmode = {
		.mq_mode = RTE_ETH_MQ_RX_RSS,					/* Enable RSS for RX queue */
		.split_hdr_size = 0,							/* No split of header and packet in diff buffers */
		.offloads = RTE_ETH_RX_OFFLOAD_CHECKSUM,
	},
	.rx_adv_conf = {
		.rss_conf = {
			.rss_key = NULL,
			.rss_hf = RTE_ETH_RSS_IP,
		},
	},
	.txmode = {
		.mq_mode = RTE_ETH_MQ_TX_NONE,
	},
};
