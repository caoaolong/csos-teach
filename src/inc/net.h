#ifndef CSOS_NET_H
#define CSOS_NET_H

#include <types.h>

#define NET_DEV_NAME_LEN    16

typedef struct e1000_t {
    char name[NET_DEV_NAME_LEN];
    uint32_t base;
    mac_addr mac;
} e1000_t;

void net_init();

#endif