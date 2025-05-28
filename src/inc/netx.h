#ifndef CSOS_NETX_H
#define CSOS_NETX_H

#include <kernel.h>
#include <list.h>
#include <netx/arp_map.h>
#include <netx/inet.h>
#include <csos/sem.h>

typedef struct desc_buff_t
{
	list_node_t node;
	uint16_t length;
	uint32_t refp;
	uint8_t payload[0];
} desc_buff_t;

// 将32位网络字节序转换为主机字节序
static inline uint32_t ntohl(uint32_t netlong) {
    return ((netlong & 0xFF000000) >> 24) | // 移动到最低位
           ((netlong & 0x00FF0000) >> 8)  | // 移动到次低位
           ((netlong & 0x0000FF00) << 8)  | // 移动到次高位
           ((netlong & 0x000000FF) << 24);  // 移动到最高位
}

// 将16位网络字节序转换为主机字节序
static inline uint16_t ntohs(uint16_t netshort) {
    return ((netshort & 0xFF00) >> 8) | // 移动到最低位
           ((netshort & 0x00FF) << 8);   // 移动到最高位
}

// 将32位主机字节序转换为网络字节序
static inline uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF000000) >> 24) |
           ((hostlong & 0x00FF0000) >> 8)  |
           ((hostlong & 0x0000FF00) << 8)  |
           ((hostlong & 0x000000FF) << 24);
}

// 将16位主机字节序转换为网络字节序
static inline uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF00) >> 8) |
           ((hostshort & 0x00FF) << 8);
}

static inline uint32_t ip2uint32(ip_addr ip) {
    return ((uint32_t)ip[0] << 24) | 
           ((uint32_t)ip[1] << 16) | 
           ((uint32_t)ip[2] << 8)  | 
           (uint32_t)ip[3];
}

void free_desc_buff(desc_buff_t *buff);
desc_buff_t *alloc_desc_buff();
void reply_desc_buff(netif_t *netif, desc_buff_t *buff, uint8_t type);
void desc_buff_init();

BOOL kernel_setmac(netif_t *netif, ip_addr ip, mac_addr mac);

uint16_t calc_checksum(uint8_t *data, uint32_t length);

netif_t *netif_default();
void netif_input(desc_buff_t *buff);
void netif_output(desc_buff_t *buff);
int netif_create(ip_addr ip, ip_addr mask, ip_addr gw, mac_addr mac);

int alloc_port(uint16_t port, socket_t *socket, uint8_t protocol);
uint16_t alloc_random_port(socket_t *socket, uint8_t protocol);
void free_port(uint16_t port);
port_t *get_port(uint16_t port);

socket_t *alloc_socket();
void free_socket(socket_t *socket);

void sys_arpl(arp_map_data_t *arp_data);
void sys_arpc();
void sys_ping(const char *ip);
void sys_ifconf(netif_dev_t *devs, int *devc);
void sys_enum_port();

int sys_socket(uint8_t family, uint8_t type, uint8_t flags);
int sys_connect(int fd, sock_addr_t *addr, uint8_t addrlen);
int sys_close(int fd);

void net_init();
void net_save();
#endif