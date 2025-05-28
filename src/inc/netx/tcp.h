#ifndef CSOS_TCP_H
#define CSOS_TCP_H

#include <types.h>
#include <netx.h>
#include <netx/ipv4.h>

enum {
    FLAGS_FIN = 0b000001,
    FLAGS_SYN = 0b000010,
    FLAGS_RST = 0b000100,
    FLAGS_PSH = 0b001000,
    FLAGS_ACK = 0b010000,
    FLAGS_URG = 0b100000
};

typedef struct tcp_t {
    uint16_t src_port;  // 源端口
    uint16_t dst_port;  // 目的端口
    uint32_t seq_num;   // 序列号
    uint32_t ack_num;   // 确认号
    union {
        uint16_t v;
        struct {
            uint16_t flags: 6; // 标志位
            uint16_t unused: 6; // 保留字段
            uint16_t offset: 4; // 数据偏移量
        };
    } ff;
    uint16_t window_size; // 窗口大小
    uint16_t checksum;  // 校验和
    uint16_t urgent_pointer; // 紧急指针
    uint8_t payload[0];
} tcp_t;

uint16_t calc_tcp_checksum(ipv4_t *ip, tcp_t *tcp, uint8_t *payload, uint16_t data_len);

void tcp_input(netif_t *netif, desc_buff_t *buff);
void tcp_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen);

void tcp_build(netif_t *netif, desc_buff_t *buff, 
    ip_addr dst_ip, uint16_t src_port, uint16_t dst_port, 
    uint8_t *data, uint16_t dlen);

void tcp_syn(socket_t *socket, desc_buff_t *buff, ip_addr dst_ip, uint16_t dst_port);
void tcp_synack(socket_t *socket, desc_buff_t *buff);
void tcp_finack(socket_t *socket, desc_buff_t *buff, ip_addr dst_ip);
void tcp_ack(socket_t *socket, desc_buff_t *buff);

#endif