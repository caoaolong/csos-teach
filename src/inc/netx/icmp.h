#ifndef CSOS_ICMP_H
#define CSOS_ICMP_H

#include <types.h>
#include <netx/ipv4.h>

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

void eth_proc_icmp(eth_t *eth, uint16_t length);

void icmp_request(e1000_t *e1000, eth_t *eth, uint8_t *data, uint16_t dlen);
void icmp_reply(e1000_t *e1000, eth_t *eth, uint8_t *data, uint16_t dlen);
void icmp_send(mac_addr dst_mac, ip_addr dst_ip);

#endif