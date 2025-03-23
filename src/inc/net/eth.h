#ifndef CSOS_ETH_H
#define CSOS_ETH_H

#include <types.h>

typedef struct eth_t {
    mac_addr dst;
    mac_addr src;
    uint16_t type;
    uint8_t payload[0];
} eth_t;

#endif