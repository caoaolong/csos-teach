#include <netx.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <netx/icmp.h>
#include <netx/udp.h>
#include <netx/tcp.h>
#include <logf.h>
#include <csos/string.h>

void ipv4_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    logf("IPv4: %02X:%d", ipv4->proto, ntohs(ipv4->total_len));
    switch (ipv4->proto)
    {
    case IP_TYPE_ICMP:
        icmp_input(netif, buff);
        break;
    case IP_TYPE_TCP:
        tcp_input(netif, buff);
        break;
    case IP_TYPE_UDP:
        udp_input(netif, buff);
        break;
    default:
        free_desc_buff(buff);
        break;
    }
}

void ipv4_build(netif_t *netif, desc_buff_t *buff, 
    ip_addr dst_ip, uint8_t tp,
    uint8_t *data, uint16_t dlen,
    uint8_t *options, uint16_t olen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    ipv4->ver = 4;
    ipv4->ihl = 5;
    uint16_t total_len = 0;
    if (tp == IP_TYPE_UDP) {
        total_len = sizeof(ipv4_t) + sizeof(udp_t) + dlen;
    } else if (tp == IP_TYPE_ICMP) {
        total_len = sizeof(ipv4_t) + sizeof(icmp_echo_t) + dlen;
    } else if (tp == IP_TYPE_TCP) {
        total_len = sizeof(ipv4_t) + sizeof(tcp_t) + dlen + olen;
    }
    ipv4->total_len = htons(total_len);
    ipv4->tos = 0;
    ipv4->flags = 0;
    ipv4->id = htons((uint16_t)xrandom());
    ipv4->ttl = 64;
    ipv4->proto = tp;
    ipv4->checksum = 0;
    kernel_memcpy(ipv4->src_ip, netif->ipv4, IPV4_LEN);
    kernel_memcpy(ipv4->dst_ip, dst_ip, IPV4_LEN);
    mac_addr dst_mac;
    kernel_setmac(netif, dst_ip, dst_mac);
    buff->length += sizeof(ipv4_t);
    eth_build(netif, buff, dst_mac, ETH_TYPE_IPv4, data, dlen);
}

void ipv4_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    uint8_t tp = ipv4->proto;
    if (tp == IP_TYPE_TCP) {
        tcp_t *tcp = (tcp_t *)ipv4->payload;
        if (tcp->ff.flags == FLAGS_ACK) {
            tcp->ff.unused = 0;
            tcp->ff.offset = sizeof(tcp_t) / 4;
            tcp->ff.v = htons(tcp->ff.v);
        }
    }
    uint16_t total_len = 0;
    if (tp == IP_TYPE_UDP) {
        total_len = sizeof(ipv4_t) + sizeof(udp_t) + dlen;
    } else if (tp == IP_TYPE_ICMP) {
        total_len = sizeof(ipv4_t) + sizeof(icmp_echo_t) + dlen;
    } else if (tp == IP_TYPE_TCP) {
        total_len = sizeof(ipv4_t) + sizeof(tcp_t) + dlen;
    }
    ipv4->total_len = htons(total_len);
    ipv4->checksum = 0;
    ipv4->id = (uint16_t)xrandom();
    ipv4->ttl = 64;
    kernel_memcpy(ipv4->dst_ip, ipv4->src_ip, IPV4_LEN);
    kernel_memcpy(ipv4->src_ip, netif->ipv4, IPV4_LEN);
    eth_output(netif, buff, data, dlen);
}