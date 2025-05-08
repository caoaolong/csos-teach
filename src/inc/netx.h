#ifndef CSOS_NETX_H
#define CSOS_NETX_H

#include <kernel.h>
#include <list.h>
#include <netx/arp_map.h>
#include <netx/inet.h>
#include <csos/sem.h>

enum {
	DBT_UNK, DBT_ARP, DBT_ICMP
};

typedef struct desc_buff_t
{
	list_node_t node;
	uint16_t length;
	uint32_t refp;
	uint8_t payload[0];
} desc_buff_t;

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
void inet_pton(const char *ipstr, ip_addr ipv);

netif_t *netif_default();
void netif_input(desc_buff_t *buff);
void netif_output(desc_buff_t *buff);
int netif_create(ip_addr ip, ip_addr mask, ip_addr gw, mac_addr mac);

void sys_arpl(arp_map_data_t *arp_data);
void sys_arpc();
void sys_ping(const char *ip);
void sys_ifconf(netif_dev_t *devs, int *devc);

void net_init();
void net_save();
#endif