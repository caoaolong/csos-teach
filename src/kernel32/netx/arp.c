#include <netx.h>
#include <netx/arp.h>
#include <csos/string.h>

void arp_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    arp_t *arp = (arp_t *)eth->payload;
    uint16_t op = ntohs(arp->op);
    if (op == ARP_OP_REPLY) {
        put_arp_map(arp->src_ip, arp->src_mac); // 保存ARP映射
        return;
    } else if (op == ARP_OP_REQUEST) {
        if (!kernel_memcmp(arp->dst_ip, netif->ipv4, IPV4_LEN)) {
            arp_output(netif, buff);
        }
        put_arp_map(arp->src_ip, arp->src_mac); // 保存ARP映射
    }
    // 释放缓冲区
    free_desc_buff(buff);
}

void arp_build(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    arp_t *arp = (arp_t *)eth->payload;
    kernel_memcpy(arp->dst_mac, "\xFF\xFF\xFF\xFF\xFF\xFF", MAC_LEN);
    kernel_memset(arp->dst_ip, 0, IPV4_LEN);
    eth_build(netif, buff, arp->dst_mac, ETH_TYPE_ARP, NULL, 0);
}

void arp_output(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    arp_t *arp = (arp_t *)eth->payload;
    arp->op = htons(ARP_OP_REPLY);
    kernel_memcpy(arp->dst_mac, arp->src_mac, MAC_LEN); // 目标MAC地址
    kernel_memcpy(arp->dst_ip, arp->src_ip, IPV4_LEN); // 目标IP地址
    kernel_memcpy(arp->src_mac, netif->mac, MAC_LEN); // 源MAC地址
    kernel_memcpy(arp->src_ip, netif->ipv4, IPV4_LEN); // 源IP地址
    eth_output(netif, buff, ETH_TYPE_ARP, NULL, 0);
}