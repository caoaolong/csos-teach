#include <netx.h>
#include <logf.h>
#include <csos/string.h>

void eth_proc_udp(eth_t *eth, uint16_t length)
{
    e1000_t *e1000 = get_e1000dev();
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    logf("UDP %d -> %d", ntohs(udp->src_port), ntohs(udp->dst_port));
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

void udp_send(ip_addr dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t *data, uint16_t dlen)
{
    // 获取目标MAC地址
    mac_addr dst_mac;
    get_mac(dst_ip, dst_mac);
    // 申请缓冲区
    e1000_t *e1000 = get_e1000dev();
    desc_buff_t *buff = alloc_desc_buff(e1000);
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    uint16_t udpl = sizeof(udp_t) + dlen;
    buff->length = sizeof(eth_t) + sizeof(ipv4_t) + udpl;
    // 构建数据包
    udp_request(eth, src_port, dst_port, data, dlen);
    ipv4_request(e1000, eth, dst_ip, IP_TYPE_UDP, NULL, 0, NULL, udpl);
    eth_request(e1000, buff, dst_mac, ETH_TYPE_IPv4);
    // 发送数据包
    e1000_send_packet(buff);
    // 释放缓冲区
    free_desc_buff(buff);
}