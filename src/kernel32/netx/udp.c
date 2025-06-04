#include <netx.h>
#include <netx/eth.h>
#include <netx/udp.h>
#include <netx/dhcp.h>
#include <logf.h>
#include <csos/string.h>

uint16_t calc_udp_checksum(ipv4_t *ip, udp_t *udp, uint8_t *payload, uint16_t data_len)
{
    uint32_t checksum = 0;
    uint16_t udp_len = sizeof(udp_t) + data_len;

    // Pseudo header (in network byte order)
    checksum += (ip->src_ip[0] << 8) | ip->src_ip[1];
    checksum += (ip->src_ip[2] << 8) | ip->src_ip[3];
    checksum += (ip->dst_ip[0] << 8) | ip->dst_ip[1];
    checksum += (ip->dst_ip[2] << 8) | ip->dst_ip[3];
    checksum += IP_TYPE_UDP;           // Protocol number (17), 8 bits
    checksum += udp_len;               // UDP length (already in host order)

    // Copy UDP header and data into buffer to process
    udp->checksum = 0;  // Must be zero before computing

    // Prepare a pointer to the entire UDP segment
    uint8_t *seg = (uint8_t *)udp;

    for (uint16_t i = 0; i < udp_len; i += 2) {
        uint16_t word = seg[i] << 8;
        if (i + 1 < udp_len) {
            word |= seg[i + 1];
        }
        checksum += word;
    }

    // Fold 32-bit sum to 16 bits
    while (checksum >> 16)
        checksum = (checksum & 0xFFFF) + (checksum >> 16);

    return htons(~checksum ? (uint16_t)(~checksum) : 0xFFFF);  // 0 is not allowed
}

void udp_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    uint16_t sp = ntohs(udp->src_port);
    uint16_t tp = ntohs(udp->dst_port);
    if (sp == DHCP_SERVER_PORT && tp == DHCP_CLIENT_PORT) {
        dhcp_input(netif, buff);
    } else {
        free_desc_buff(buff);
    }
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
    udp->src_port = htons(src_port);
    udp->dst_port = htons(dst_port);
    udp->length = htons(sizeof(udp_t) + dlen);
    udp->checksum = 0;
    buff->length += sizeof(udp_t);
    ipv4_build(netif, buff, dst_ip, IP_TYPE_UDP, data, dlen, NULL, 0);
}