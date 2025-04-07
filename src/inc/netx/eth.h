#ifndef CSOS_NETX_H
#define CSOS_NETX_H

#include <types.h>

// 以太网帧
typedef struct eth_t
{
    mac_addr dst; // 目标地址
    mac_addr src; // 源地址
    uint16_t type;       // 类型
    uint8_t payload[0];  // 载荷
} eth_t;

#endif