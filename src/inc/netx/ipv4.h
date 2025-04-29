#ifndef CSOS_IPV4_H
#define CSOS_IPV4_H

#include <types.h>
#include <netx/eth.h>

#define IP_TYPE_ICMP    1
#define IP_TYPE_TCP     6
#define IP_TYPE_UDP     17

typedef struct ipv4_t {
    uint8_t ihl:4;  // 首部长度
    uint8_t ver:4; // 版本
    uint8_t tos;   // 服务类型
    uint16_t total_len; // 总长度
    uint16_t id;   // 标识
    union {
        struct {
            uint8_t  flag:3; // 标志
            uint16_t frag_off:13; // 片偏移
        };
        uint16_t flags;
    };
    uint8_t  ttl;   // 生存时间
    uint8_t  proto; // 协议
    uint16_t checksum; // 校验和
    ip_addr src_ip; // 源IP地址
    ip_addr dst_ip; // 目标IP地址
    uint8_t payload[0]; // 数据负载
} ipv4_t;

void ipv4_input(netif_t *netif, desc_buff_t *buff);
void ipv4_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen);

void ipv4_build(netif_t *netif, desc_buff_t *buff, 
    ip_addr dst_ip, uint8_t tp,
    uint8_t *data, uint16_t dlen);

#endif