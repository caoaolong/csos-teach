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

enum {
    PORT_DOWN, PORT_BUSY, PORT_UP
};

enum {
	DBT_UNK, DBT_ARP, DBT_ICMP, DBT_UDP, DBT_TCP
};

typedef struct port_t {
    uint8_t  status:4;   // 端口状态
    uint8_t  ifid:4;     // 网络接口ID
    uint8_t  ptype;      // 端口协议类型
    uint16_t pid;        // 端口对应的进程ID
} port_t;

void arpl(arp_map_data_t *arp_data);
void arpc();
void ping(const char *ip);
void ifconf(netif_dev_t *devs, int *devc);
void enum_port(port_t *port, uint16_t *cp, uint16_t *np);

#endif