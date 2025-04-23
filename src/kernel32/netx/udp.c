#include <netx.h>
#include <csos/string.h>

void eth_proc_udp(eth_t *eth, uint16_t length)
{
    e1000_t *e1000 = get_e1000dev();
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    if (htons(udp->src_port) == PORT_DHCP_SERVER && htons(udp->dst_port) == PORT_DHCP_CLIENT) {
        eth_proc_dhcp(eth, udp, length);
    }
}

void udp_request(eth_t *eth, uint16_t sp, uint16_t tp, uint8_t *data, uint16_t dlen)
{
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    udp->src_port = htons(sp);
    udp->dst_port = htons(tp);
    udp->length = htons(sizeof(udp_t) + dlen);
    if (data) 
        kernel_memcpy(udp->payload, data, dlen);
    udp->checksum = calc_checksum((uint8_t *)udp, sizeof(udp_t) + dlen);
}

void udp_reply(eth_t *eth, uint8_t *data, uint16_t dlen)
{
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    uint16_t tp = udp->src_port;
    uint16_t sp = udp->dst_port;
    udp->src_port = sp;
    udp->dst_port = tp;
    udp->length = htons(sizeof(udp_t) + dlen);
    if(data) 
        kernel_memcpy(udp->payload, data, dlen);
    udp->checksum = calc_checksum((uint8_t *)udp, udp->length);
}

void udp_send(ip_addr dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t *data, uint16_t length)
{
    // TODO: 获取目标MAC地址
    mac_addr dst_mac;
    // 申请缓冲区
    e1000_t *e1000 = get_e1000dev();
    desc_buff_t *buff = alloc_desc_buff(e1000);
    // 构建数据包
    eth_t *eth = (eth_t *)buff->payload;
    eth_request(e1000, buff, dst_mac, ETH_TYPE_IPv4);
    buff->length += sizeof(eth_t);
    ipv4_request(e1000, eth, dst_ip, IP_TYPE_ICMP, NULL, 0, NULL, 0);
    buff->length += sizeof(ipv4_t);
    udp_request(eth, src_port, dst_port, data, length);
    buff->length += (sizeof(udp_t) + length);
    // 更新IPv4总长度
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    ipv4->total_len = htons(sizeof(ipv4_t) + sizeof(udp_t) + sizeof(length));
    // 发送数据包
    e1000_send_packet(buff);
    // 释放缓冲区
    free_desc_buff(e1000, buff);
}