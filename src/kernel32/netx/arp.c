#include <netx.h>
#include <pci/e1000.h>
#include <csos/string.h>

eth_t *arp_request(ip_addr ip)
{
    eth_t *eth = eth_frame("\xFF\xFF\xFF\xFF\xFF\xFF", ETH_TYPE_ARP);
    arp_t *arp = (arp_t *)eth->payload;
    arp->hw_type = htons(0x0001); // 硬件类型
    arp->proto_type = htons(ETH_TYPE_IPv4); // 协议类型
    arp->hw_size = MAC_LEN; // 硬件地址长度
    arp->proto_size = IPV4_LEN; // 协议地址长度
    arp->op = htons(ARP_OP_REQUEST); // 操作码
    kernel_memcpy(arp->src_mac, eth->src, MAC_LEN); // 源MAC地址
    kernel_memcpy(arp->src_ip, OS_IPv4, IPV4_LEN); // 源IP地址
    kernel_memcpy(arp->dst_mac, "\x00\x00\x00\x00\x00\x00", MAC_LEN); // 目标MAC地址
    kernel_memcpy(arp->dst_ip, ip, IPV4_LEN); // 目标IP地址
    return eth;
}

eth_t *arp_replay(eth_t *eth)
{
    arp_t *arp = (arp_t *)eth->payload;
    arp->op = htons(ARP_OP_REPLY); // 操作码
    // 保存发送地址
    mac_addr target_mac;
    ip_addr target_ip;
    kernel_memcpy(target_mac, arp->src_mac, MAC_LEN);
    kernel_memcpy(target_ip, arp->src_ip, IPV4_LEN);
    // 构建数据包
    kernel_memcpy(arp->dst_mac, target_mac, MAC_LEN);
    kernel_memcpy(arp->dst_ip, target_ip, IPV4_LEN);
    e1000_t *e1000 = get_e1000dev();
    kernel_memcpy(arp->src_mac, e1000->mac, MAC_LEN);
    kernel_memcpy(arp->src_ip, e1000->ipv4, IPV4_LEN);
    return eth;
}