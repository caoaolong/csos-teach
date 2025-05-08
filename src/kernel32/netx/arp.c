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
        reply_desc_buff(netif, buff, DBT_ARP);
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

void arp_build(netif_t *netif, desc_buff_t *buff, ip_addr dst_ip)
{
    eth_t *eth = (eth_t *)buff->payload;
    arp_t *arp = (arp_t *)eth->payload;
    arp->hw_type = htons(1);
    arp->hw_size = 6;
    arp->proto_type = htons(ETH_TYPE_IPv4);
    arp->proto_size = 4;
    arp->op = htons(ARP_OP_REQUEST);
    kernel_memset(arp->dst_mac, 0, MAC_LEN); // 目标MAC地址
    kernel_memcpy(arp->dst_ip, dst_ip, IPV4_LEN); // 目标IP地址
    kernel_memcpy(arp->src_mac, netif->mac, MAC_LEN); // 源MAC地址
    kernel_memcpy(arp->src_ip, netif->ipv4, IPV4_LEN); // 源IP地址
    eth_build(netif, buff, "\xFF\xFF\xFF\xFF\xFF\xFF", ETH_TYPE_ARP, NULL, sizeof(arp_t));
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