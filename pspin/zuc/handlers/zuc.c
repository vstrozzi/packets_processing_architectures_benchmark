// Copyright 2020 ETH Zurich
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <handler.h>
#include <packets.h>
#include <spin_dma.h>
#include <stdlib.h>

#include "zuc.h"

#if !defined(FROM_L2) && !defined(FROM_L1)
#define FROM_L1
#endif

/*  Payload handler: all packets */
__handler__ void zuc_ph(handler_args_t *args)
{
    // Take args
    task_t* task = args->task;
    // Get instr pointer (address) in L1 packet memory of the ip packet
    ip_hdr_t *ip_hdr = (ip_hdr_t*) (task->pkt_mem);
#ifdef FROM_L2
    // I.e. the packet memory is in L2, save respective address
    uint8_t *nic_pld_addr = ((uint8_t*) (task->l2_pkt_mem)); 
#else
    // I.e. the packet memory is in L1, save respective address (same address as ip_hdr)
    uint8_t *nic_pld_addr = ((uint8_t*) (task->pkt_mem)); 
#endif

    // Packet length and payload
    uint16_t pkt_pld_len = ip_hdr->length;
    uint8_t *pkt_pld_ptr;

    // Fill pkt_pld_ptr and pkt_pld_len based on our pkt_address
    GET_IP_UDP_PLD(task->pkt_mem, pkt_pld_ptr, pkt_pld_len);

    // Define udp_header at some location, by taking care not to overwrite existing memory
    // I.e. we take the pkt_mem and move 4 times for ihl (length of ip-header) 4 times
    udp_hdr_t *udp_hdr = (udp_hdr_t*) (((uint8_t*) (task->pkt_mem)) + ip_hdr->ihl * 4);

    // Get UDP-payload's length
    uint32_t payload_size = __ntohs(udp_hdr->length) - sizeof(udp_hdr_t);


    // Start Encrypting/Decrypting, by taking the IV stored in the first 128 bits of the payload
    // and the remaining message
    uint32_t *IV = &(((uint32_t *) pkt_pld_ptr)[0]);
    uint32_t LENGTH = pkt_pld_len - sizeof(uint32_t);
    uint32_t *M = ((uint32_t *) pkt_pld_ptr) + 1;


    // Allocate a new position for the Encrypted message in L1 memory (1 Mib per cluster)
    // We assume a max length of message payload of 1500 B, hence we can have a space
    // of 1500 in between HPU memory (3000 in total since we allocate another var in zuc.h)
    uint32_t *C = (uint32_t *) (((uint8_t*) (task->scratchpad[args->cluster_id])) + 2*1500*args->hpu_id);

    // Encrypt and Decrypt
    EEA3((uint8_t *) IV, LENGTH, M, C,  args);
    // Write resulting-data to existing packet
    spin__memcpy(C, M, LENGTH);

    // Switch source and destination id to send it back
    uint32_t src_id = ip_hdr->source_id;
    ip_hdr->source_id  = ip_hdr->dest_id;
    ip_hdr->dest_id  = src_id;

    // Switch source and destination port to send it back
    uint16_t src_port = udp_hdr->src_port;
    udp_hdr->src_port = udp_hdr->dst_port;
    udp_hdr->dst_port = src_port;

    // Define put argument
    spin_cmd_t put;
    // Send packet 
    spin_send_packet(nic_pld_addr, pkt_pld_len, &put);

    //It's not strictly necessary to wait. The hw will enforce that the feedback is not
    //sent until all commands issued by this handlers are completed.
#ifdef WAIT_POLL
    bool completed = false;
    do {
        spin_cmd_test(put, &completed);
    } while (!completed);
#elif defined(WAIT_SUSPEND)
    spin_cmd_wait(put);
#endif

}

void init_handlers(handler_fn * hh, handler_fn *ph, handler_fn *th, void **handler_mem_ptr)
{
    volatile handler_fn handlers[] = {NULL, zuc_ph, NULL};
    // All handlers are optional
    *hh = handlers[0]; // Header handler i.e. first packer
    *ph = handlers[1]; // Payload Handler i.e. all packets
    *th = handlers[2]; // Completion Handler i.e. after all packets have been processed
}
