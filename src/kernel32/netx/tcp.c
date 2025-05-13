#include <netx/tcp.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <csos/string.h>

#define TCP_TEST_PORT 8000

uint16_t calc_tcp_checksum(ipv4_t *ip, tcp_t *tcp, uint8_t *payload, uint16_t data_len)
{
    uint32_t checksum = 0;
    // 从 tcp->offset 字段提取 TCP 头部长度（高 4 位）
    uint16_t tcp_hdr_len = ntohs(ip->total_len) - ip->ihl * 4;
    uint16_t tcp_len = tcp_hdr_len + data_len;

    // 1. Pseudo header
    checksum += (ip->src_ip[0] << 8) | ip->src_ip[1];
    checksum += (ip->src_ip[2] << 8) | ip->src_ip[3];
    checksum += (ip->dst_ip[0] << 8) | ip->dst_ip[1];
    checksum += (ip->dst_ip[2] << 8) | ip->dst_ip[3];
    checksum += IP_TYPE_TCP;     // Protocol number (6), 8 bits
    checksum += tcp_len;         // TCP length (already in host order)

    // 2. TCP header
    tcp->checksum = 0;
    uint8_t *tcp_bytes = (uint8_t *)tcp;
    for (uint16_t i = 0; i < sizeof(tcp_t); i += 2) {
        uint16_t word = tcp_bytes[i] << 8;
        if (i + 1 < tcp_hdr_len) {
            word |= tcp_bytes[i + 1];
        }
        checksum += word;
    }

    // 3. TCP data
    for (uint16_t i = 0; i < data_len; i += 2) {
        uint16_t word = payload[i] << 8;
        if (i + 1 < data_len) {
            word |= payload[i + 1];
        }
        checksum += word;
    }

    // 4. Fold high bits into low 16 bits
    while (checksum >> 16)
        checksum = (checksum & 0xFFFF) + (checksum >> 16);

    return htons(~checksum ? (uint16_t)(~checksum) : 0xFFFF);  // 0 is not allowed
}

void tcp_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    if (tcp->dst_port == htons(TCP_TEST_PORT)) {
        tcp->ff.v = ntohs(tcp->ff.v);
        if (tcp->ff.flags == FLAGS_SYN | FLAGS_ACK) {
            tcp_ack(netif, buff);
        } else if(tcp->ff.flags == FLAGS_SYN) {
            tcp_synack(netif, buff);
        } else if(tcp->ff.flags == FLAGS_ACK) {
            // 处理ACK包
        }
    }
}

void tcp_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
}

void tcp_build(netif_t *netif, desc_buff_t *buff, 
    ip_addr dst_ip, uint16_t src_port, uint16_t dst_port, 
    uint8_t *data, uint16_t dlen)
{

}

void tcp_syn(netif_t *netif, desc_buff_t *buff, 
    ip_addr dst_ip, uint16_t src_port, uint16_t dst_port)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    tcp->src_port = htons(TCP_TEST_PORT);
    tcp->dst_port = htons(TCP_TEST_PORT);
    tcp->seq_num = htonl(xrandom());
    tcp->ack_num = 0;
    tcp->ff.flags = FLAGS_SYN;
    tcp->ff.unused = 0;
    tcp->ff.offset = sizeof(tcp_t) / 4;
    tcp->ff.v = htons(tcp->ff.v);
    tcp->window_size = htons(0xFF);
    tcp->checksum = 0;
    tcp->urgent_pointer = 0;
    buff->length += sizeof(tcp_t);
    ipv4_build(netif, buff, dst_ip, IP_TYPE_TCP, NULL, 0);
}

void tcp_synack(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    // 接收到的数据包的src_port和dst_port需要交换
    uint16_t dst_port = tcp->src_port;
    tcp->src_port = tcp->dst_port;
    tcp->dst_port = dst_port;
    tcp->seq_num++;
    tcp->ack_num = htonl(xrandom());
    tcp->ff.flags = FLAGS_SYN | FLAGS_ACK;
    tcp->ff.unused = 0;
    tcp->ff.offset = sizeof(tcp_t) / 4;
    tcp->ff.v = htons(tcp->ff.v);
    tcp->checksum = 0;
    ipv4_output(netif, buff, NULL, 0);
}

void tcp_ack(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    // 接收到的数据包的src_port和dst_port需要交换
    uint16_t dst_port = tcp->src_port;
    tcp->src_port = tcp->dst_port;
    tcp->dst_port = dst_port;
    uint32_t seq_num = ntohl(tcp->seq_num);
    tcp->seq_num = tcp->ack_num;
    tcp->ack_num = htonl(seq_num + 1);
    tcp->ff.flags = FLAGS_ACK;
    tcp->window_size = htons(0xFF);
    tcp->checksum = 0;
    ipv4_output(netif, buff, NULL, 0);
}