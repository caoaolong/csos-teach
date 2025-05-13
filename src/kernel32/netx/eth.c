#include <netx.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <netx/udp.h>
#include <netx/icmp.h>
#include <netx/tcp.h>
#include <logf.h>
#include <csos/string.h>

static void netif_set_payload(desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    uint8_t *pd = NULL;
    eth_t *eth = (eth_t *)buff->payload;
    if (ntohs(eth->type) == ETH_TYPE_IPv4) {
        ipv4_t *ipv4 = (ipv4_t *)eth->payload;
        uint16_t ipv4l = ipv4->ihl * 4;
        pd = (uint8_t *)ipv4;
        if (ipv4->proto == IP_TYPE_ICMP) {
            icmp_t *icmp = (icmp_t *)(pd + ipv4l);
            if ((icmp->type == ICMP_TYPE_ECHO_REPLY || icmp->type == ICMP_TYPE_ECHO_REQUEST) && icmp->code == 0) {
                icmp_echo_t *echo = (icmp_echo_t *)icmp;
                kernel_memcpy(echo->payload, data, dlen);
            }
        } else if (ipv4->proto == IP_TYPE_UDP) {
            udp_t *udp = (udp_t *)(pd + ipv4l);
            kernel_memcpy(udp->payload, data, dlen);
        } else if (ipv4->proto == IP_TYPE_TCP) {
            tcp_t *tcp = (tcp_t *)(pd + ipv4l);
            kernel_memcpy(tcp->payload, data, dlen);
        }
    }
    buff->length += dlen;
}

static void netif_set_checksum(desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    uint8_t *pd = NULL;
    eth_t *eth = (eth_t *)buff->payload;
    if (ntohs(eth->type) == ETH_TYPE_IPv4) {
        // IPv4检验和
        ipv4_t *ipv4 = (ipv4_t *)eth->payload;
        uint16_t ipv4l = ipv4->ihl * 4;
        pd = (uint8_t *)ipv4;
        ipv4->checksum = calc_checksum(pd, ipv4l);
        if (ipv4->proto == IP_TYPE_ICMP) {
            pd += ipv4l;
            icmp_t *icmp = (icmp_t *)pd;
            if ((icmp->type == ICMP_TYPE_ECHO_REPLY || icmp->type == ICMP_TYPE_ECHO_REQUEST) && icmp->code == 0) {
                icmp->checksum = calc_checksum(pd, sizeof(icmp_echo_t) + dlen);
            }
        } else if (ipv4->proto == IP_TYPE_UDP) {
            pd += ipv4l;
            udp_t *udp = (udp_t *)pd;
            udp->checksum = calc_udp_checksum(ipv4, udp, udp->payload, dlen);
        } else if (ipv4->proto == IP_TYPE_TCP) {
            pd += ipv4l;
            tcp_t *tcp = (tcp_t *)pd;
            tcp->checksum = calc_tcp_checksum(ipv4, tcp, tcp->payload, dlen);
        }
    }
}

void eth_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    logf("Receive packet: %d bytes", buff->length);
    if (kernel_memcmp(eth->dst, netif->mac, MAC_LEN) && kernel_memcmp(eth->dst, "\xFF\xFF\xFF\xFF\xFF\xFF", MAC_LEN)) {
        free_desc_buff(buff);
        return;
    }
    switch (ntohs(eth->type)) {
        case ETH_TYPE_ARP:
            arp_input(netif, buff);
            break;
        case ETH_TYPE_IPv4:
            ipv4_input(netif, buff);
            break;
        case ETH_TYPE_IPv6:
            break;
        case ETH_TYPE_TEST:
            logf("Ethernet Configuration Testing");
            break;
        default:
            free_desc_buff(buff);
            break;
    }
}

void eth_build(netif_t *netif, desc_buff_t *buff, 
    mac_addr dst_mac, uint16_t type, 
    uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    eth->type = htons(type);
    kernel_memcpy(eth->dst, dst_mac, MAC_LEN);
    kernel_memcpy(eth->src, netif->mac, MAC_LEN);
    buff->length += sizeof(eth_t);
    netif_set_payload(buff, data, dlen);
    netif_set_checksum(buff, data, dlen);
    netif_output(buff);
}

void eth_output(netif_t *ifnet, desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    kernel_memcpy(eth->dst, eth->src, MAC_LEN);
    kernel_memcpy(eth->src, ifnet->mac, MAC_LEN);
    netif_set_payload(buff, data, dlen);
    netif_set_checksum(buff, data, dlen);
    netif_output(buff);
}