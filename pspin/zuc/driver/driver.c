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

#include <stdint.h>
#include "gdriver.h"
#include "packets.h"

#define FIX 1024
#define SEED 236695

// Function used to randomly generate (fixed seed) the packet content, with 32 bits values in a range of [0, 1024]
uint32_t fill_packet(uint32_t msg_idx, uint32_t pkt_idx, uint8_t *pkt_buff, uint32_t max_pkt_size, uint32_t* l1_pkt_size)
{   
    pkt_hdr_t *hdr = (pkt_hdr_t*) pkt_buff;
    hdr->ip_hdr.ihl = 5;
    hdr->ip_hdr.length = max_pkt_size;

    uint32_t payload_len = max_pkt_size - sizeof(pkt_hdr_t);
    uint32_t *payload_ptr =  (uint32_t *) (pkt_buff + sizeof(pkt_hdr_t));

    for (int i=0; i<payload_len/sizeof(uint32_t); i++)
    {
        payload_ptr[i] = rand() % FIX;
    }

    return max_pkt_size;
}

int main(int argc, char**argv)
{
    // Initialize handlers of file
    const char *handlers_file="build/zuc";
    const char *hh=NULL;
    const char *ph="zuc_ph";
    const char *th=NULL;

    // Use driver already present using the file of ping_pong.h
    gdriver_init(argc, argv, handlers_file, hh, ph, th);
    gdriver_set_packet_fill_callback(fill_packet);
    
    gdriver_run();

    gdriver_fini();
    return 0;
}
