#ifndef CSOS_ARP_H
#define CSOS_ARP_H

#include <types.h>
#include <netx/eth.h>
#include <netx/arp_map.h>

#define ARP_OP_REQUEST 0x0001 // ARP请求
#define ARP_OP_REPLY   0x0002 // ARP应答

typedef struct arp_t {
    uint16_t hw_type; // 硬件类型
    uint16_t proto_type; // 协议类型
    uint8_t hw_size; // 硬件地址长度
    uint8_t proto_size; // 协议地址长度
    uint16_t op; // 操作码
    mac_addr src_mac; // 源MAC地址
    ip_addr src_ip; // 源IP地址
    mac_addr dst_mac; // 目标MAC地址
    ip_addr dst_ip; // 目标IP地址
} arp_t;

void arp_input(netif_t *netif, desc_buff_t *buff);
void arp_output(netif_t *netif, desc_buff_t *buff);

void arp_build(netif_t *netif, desc_buff_t *buff);

#endif