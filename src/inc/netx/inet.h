#ifndef CSOS_INET_H
#define CSOS_INET_H

#include <netx/arp_map.h>
#include <netx/inet.h>

enum {
    NETIF_STATUS_DOWN,
    NETIF_STATUS_REQUESTED,
    NETIF_STATUS_ACK
};

typedef struct netif_dev_t {
    char name[8];
    ip_addr ipv4;
    ip_addr gwv4;
    ip_addr mask;
    mac_addr mac;
    uint8_t status;
} netif_dev_t;

void arpl(arp_map_data_t *arp_data);
void arpc();
void ping(const char *ip);
void ifconf(netif_dev_t *devs, int *devc);

#endif