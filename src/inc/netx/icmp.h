#ifndef CSOS_ICMP_H
#define CSOS_ICMP_H

#include <types.h>
#include <netx.h>

#define ICMP_TYPE_ECHO_REPLY        0 // 回显应答
#define ICMP_TYPE_DEST_UNREACH      3 // 目的不可达
#define ICMP_TYPE_ECHO_REQUEST      8 // 回显请求
#define ICMP_TYPE_TIME_EXCEED       11 // 超时
#define ICMP_TYPE_PARAM_PROB        12 // 参数问题

typedef struct icmp_t {
    uint8_t type; // 类型
    uint8_t code; // 代码
    uint16_t checksum; // 校验和
    uint8_t payload[0]; // 数据负载
} icmp_t;

typedef struct icmp_echo_t {
    uint8_t type; // 类型
    uint8_t code; // 代码
    uint16_t checksum; // 校验和
    uint16_t id;
    uint16_t seq;
    uint8_t payload[0]; // 数据负载
} icmp_echo_t;

void icmp_input(netif_t *netif, desc_buff_t *buff);
void icmp_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen);

void icmp_build(netif_t *netif, desc_buff_t *buff, 
    uint8_t type, uint8_t code, 
    ip_addr dst_ip,
    uint8_t *data, uint16_t dlen);

#endif