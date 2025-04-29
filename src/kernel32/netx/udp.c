#include <netx.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <netx/udp.h>
#include <logf.h>
#include <csos/string.h>

void udp_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    logf("UDP %d -> %d", ntohs(udp->src_port), ntohs(udp->dst_port));
    free_desc_buff(buff);
}

void udp_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    uint16_t tp = udp->src_port;
    uint16_t sp = udp->dst_port;
    udp->src_port = sp;
    udp->dst_port = tp;
    udp->length = htons(sizeof(udp_t) + dlen);
    udp->checksum = 0;
    buff->length += sizeof(udp_t);
    ipv4_output(netif, buff, data, dlen);
}

void udp_build(netif_t *netif, desc_buff_t *buff, 
    ip_addr dst_ip, uint16_t src_port, uint16_t dst_port, 
    uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    uint16_t tp = udp->src_port;
    uint16_t sp = udp->dst_port;
    udp->src_port = src_port;
    udp->dst_port = dst_port;
    udp->length = htons(sizeof(udp_t) + dlen);
    udp->checksum = 0;
    buff->length += sizeof(udp_t);
    ipv4_build(netif, buff, dst_ip, IP_TYPE_UDP, data, dlen);
}