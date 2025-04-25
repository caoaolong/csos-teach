#ifndef CSOS_UDP_H
#define CSOS_UDP_H

#include <types.h>
#include <netx/eth.h>

typedef struct udp_t {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t payload[0];
} udp_t;

void eth_proc_udp(eth_t *eth, uint16_t length);

void udp_request(eth_t *eth, uint16_t sp, uint16_t tp, uint8_t *data, uint16_t dlen);
void udp_reply(eth_t *eth, uint8_t *data, uint16_t dlen);
void udp_send(ip_addr dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t *data, uint16_t length);

#endif