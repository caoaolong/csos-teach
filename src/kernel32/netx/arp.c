#include <netx.h>
#include <pci/e1000.h>
#include <csos/string.h>

void eth_proc_arp(eth_t *eth, uint16_t length)
{
    e1000_t *e1000 = get_e1000dev();
    arp_t *arp = (arp_t *)eth->payload;

    uint16_t op = ntohs(arp->op);
    if (op == ARP_OP_REPLY) {
        put_arp_map(arp->src_ip, arp->src_mac); // 保存ARP映射
        return;
    }

    if (op == ARP_OP_REQUEST) {
        if (!kernel_memcmp(arp->dst_ip, e1000->ipv4, IPV4_LEN)) {
            desc_buff_t *buff = alloc_desc_buff(e1000);
            buff->length = length;
            // 复制数据包
            kernel_memcpy(buff->payload, eth, length); 
            // 构建应答包
            eth_reply(e1000, buff, eth); 
            arp_replay(e1000, (eth_t *)buff->payload);
            // 发送ARP应答
            e1000_send_packet(buff);
             // 释放缓冲区
            free_desc_buff(e1000, buff);
        }
        put_arp_map(arp->src_ip, arp->src_mac); // 保存ARP映射
    }
}

void arp_request(e1000_t *e1000, eth_t *eth, ip_addr ip)
{
    arp_t *arp = (arp_t *)eth->payload;
    arp->hw_type = htons(0x0001); // 硬件类型
    arp->proto_type = htons(ETH_TYPE_IPv4); // 协议类型
    arp->hw_size = MAC_LEN; // 硬件地址长度
    arp->proto_size = IPV4_LEN; // 协议地址长度
    arp->op = htons(ARP_OP_REQUEST); // 操作码
    kernel_memcpy(arp->src_mac, eth->src, MAC_LEN); // 源MAC地址
    kernel_memcpy(arp->src_ip, e1000->ipv4, IPV4_LEN); // 源IP地址
    kernel_memcpy(arp->dst_mac, "\x00\x00\x00\x00\x00\x00", MAC_LEN); // 目标MAC地址
    kernel_memcpy(arp->dst_ip, ip, IPV4_LEN); // 目标IP地址
}

void arp_replay(e1000_t *e1000, eth_t *eth)
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
    kernel_memcpy(arp->src_mac, e1000->mac, MAC_LEN);
    kernel_memcpy(arp->src_ip, e1000->ipv4, IPV4_LEN);
}

void arp_send(ip_addr ip)
{
    // 申请缓冲区
    e1000_t *e1000 = get_e1000dev();
    desc_buff_t *buff = alloc_desc_buff(e1000);
    // 构建数据包
    eth_request(e1000, buff, "\xFF\xFF\xFF\xFF\xFF\xFF", ETH_TYPE_ARP);
    buff->length += sizeof(eth_t);
    eth_t *eth = (eth_t *)buff->payload;
    arp_request(e1000, eth, ip);
    buff->length += sizeof(arp_t);
    // 发送数据包
    e1000_send_packet(buff);
    // 释放缓冲区
    free_desc_buff(e1000, buff);
}