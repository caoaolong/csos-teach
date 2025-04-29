#ifndef CSOS_UDP_H
#define CSOS_UDP_H

#include <types.h>
#include <netx/eth.h>
#include <netx/ipv4.h>

typedef struct udp_t {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t payload[0];
} udp_t;

uint16_t calc_udp_checksum(ipv4_t *ip, udp_t *udp, uint8_t *payload, uint16_t len);

void udp_input(netif_t *netif, desc_buff_t *buff);
void udp_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen);

void udp_build(netif_t *netif, desc_buff_t *buff, 
    ip_addr dst_ip, uint16_t src_port, uint16_t dst_port, 
    uint8_t *data, uint16_t dlen);

#endif