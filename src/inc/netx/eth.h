#ifndef CSOS_NETX_ETH_H
#define CSOS_NETX_ETH_H

#include <types.h>
#include <netx.h>

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

void eth_input(netif_t *netif, desc_buff_t *buff);
void eth_output(netif_t *ifnet, desc_buff_t *buff, uint16_t tp, uint8_t *data, uint16_t dlen);

void eth_build(netif_t *netif, desc_buff_t *buff, 
    mac_addr dst_mac, uint16_t type, 
    uint8_t *data, uint16_t dlen);

#endif