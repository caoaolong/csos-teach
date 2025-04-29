#include <netx.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <netx/icmp.h>
#include <logf.h>
#include <csos/string.h>

static char echo_payload[] = "csos.icmp.abcdefghijklmnopqrstuvwxyz0123456789";

void icmp_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    icmp_t *icmp = (icmp_t *)ipv4->payload;
    if (icmp->type == ICMP_TYPE_ECHO_REQUEST) {
        icmp_output(netif, buff, NULL, 0);
    } else if (icmp->type == ICMP_TYPE_ECHO_REPLY) {
        logf("ICMPv4 Reply: from: %d.%d.%d.%d, length: %d, ttl: %d",
            ipv4->src_ip[0], ipv4->src_ip[1], ipv4->src_ip[2], ipv4->src_ip[3],
            ntohs(ipv4->total_len), ipv4->ttl);
    }
    // 释放缓冲区
    free_desc_buff(buff);
}

void icmp_build(netif_t *netif, desc_buff_t *buff, 
    uint8_t type, uint8_t code, 
    ip_addr dst_ip,
    uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    icmp_t *icmp = (icmp_t *)ipv4->payload;
    icmp->type = type;
    icmp->code = code;
    icmp->checksum = 0;
    buff->length += sizeof(icmp_t);
    if (data == NULL && dlen == 0) {
        ipv4_build(netif, buff, dst_ip, IP_TYPE_ICMP, echo_payload, sizeof(echo_payload));
    } else {
        ipv4_build(netif, buff, dst_ip, IP_TYPE_ICMP, data, dlen);
    }
}

void icmp_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    icmp_t *icmp = (icmp_t *)ipv4->payload;
    if (icmp->type == ICMP_TYPE_ECHO_REQUEST && icmp->code == 0) {
        icmp->type = ICMP_TYPE_ECHO_REPLY;
        icmp->code = 0;
    }
    icmp->checksum = 0; // 校验和
    ipv4_output(netif, buff, data, dlen);
}