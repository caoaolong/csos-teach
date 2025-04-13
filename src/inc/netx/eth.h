#ifndef CSOS_NETX_ETH_H
#define CSOS_NETX_ETH_H

#include <types.h>
#include <netx/arp.h>

#define ETH_TYPE_IPv4   0x0800
#define ETH_TYPE_ARP    0x0806
#define ETH_TYPE_IPv6   0x86DD
#define ETH_TYPE_TEST   0x9000

// 以太网帧
typedef struct eth_t
{
    mac_addr dst; // 目标地址
    mac_addr src; // 源地址
    uint16_t type;       // 类型
    uint8_t payload[0];  // 载荷
} eth_t;

arp_map_t *get_arp_map();
void put_arp_map(ip_addr ip, mac_addr mac);
void flush_arp_map();
void eth_proc_arp(arp_t *arp, uint16_t length);

void arp_map_init();

#endif