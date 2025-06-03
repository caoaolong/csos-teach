#ifndef CSOS_INET_H
#define CSOS_INET_H

#include <list.h>
#include <netx/arp_map.h>

#define AF_INET         2
// TCP
#define SOCK_STREAM     1
// UDP
#define SOCK_DGRAM      2

enum {
    TCP_CLOSED = 0,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_CLOSING,
    TCP_TIME_WAIT
};

enum {
    SOCK_CLIENT,
    SOCK_SERVER
};

typedef struct netif_t {
    char name[8];

    mac_addr mac; // 网卡MAC地址
    mac_addr gw_mac; // 网关MAC地址
    
    ip_addr ipv4; // 网卡IP地址
    ip_addr mask; // 子网掩码
    ip_addr gw;   // 网关IPv4地址

    ip_addr dhcp_ipv4;
    ip_addr dhcp_mask;
    ip_addr dhcp_gw;

    list_t rx_list; // 接收缓冲区链表
    list_t tx_list; // 发送缓冲区链表
    list_t wait_list; // 等待队列

    uint8_t index:4;
    uint8_t status:4;

    uint8_t timer:4; // 定时器
    uint8_t period:4; // 周期

    char unused[58];
} netif_t;

typedef struct socket_t {
    uint8_t exists;
    uint8_t socktype;
    uint16_t backlog;
    uint8_t family;
    uint8_t type;
    uint8_t flags;
    uint8_t state;
    uint16_t srcp;
    uint16_t dstp;
    ip_addr dipv4;
    ip_addr sipv4;
    uint32_t seq;
    uint32_t ack;
    uint32_t fp;       // 文件指针
    netif_t *netif;    // 绑定的网络接口
} socket_t;

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
    PORT_DOWN, PORT_BUSY, PORT_UP, PORT_LISTEN
};

enum {
	DBT_UNK, DBT_ARP, DBT_ICMP, DBT_UDP, DBT_TCP
};

typedef struct port_t {
    uint8_t  status;     // 端口状态
    uint8_t  ptype;      // 端口协议类型
    uint16_t pid;        // 端口对应的进程ID
    socket_t *sock;      // 端口对应的socket
} port_t;

void arpl(arp_map_data_t *arp_data);
void arpc();
void ping(const char *ip);
void ifconf(netif_dev_t *devs, int *devc);
void enum_port(port_t *port, uint16_t *cp, uint16_t *np);

int socket(uint16_t family, uint8_t type, uint8_t flags);
int connect(int fd, sock_addr_t addr, uint8_t addrlen);
int close(int fd);

int bind(int fd, sock_addr_t addr, uint8_t addrlen);
int listen(int fd, uint16_t backlog);
int accept(int fd, sock_addr_t *addr, uint8_t *addrlen);

void inet_pton(const char *ipstr, ip_addr ipv);

#endif