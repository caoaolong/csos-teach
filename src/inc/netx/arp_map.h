#ifndef CSOS_ARP_MAP_H
#define CSOS_ARP_MAP_H

#include <types.h>

#define ARP_MAP_NR  51 // ARP映射数量

typedef struct arp_map_item_t {
    ip_addr ip; // IP地址
    mac_addr mac; // MAC地址
} arp_map_item_t;

typedef struct arp_map_data_t {
    arp_map_item_t items[ARP_MAP_NR]; // ARP映射表项
    uint16_t idx; // 当前索引
} arp_map_data_t;

typedef struct arp_map_t {
    arp_map_data_t data; // ARP映射数据
    uint8_t dirty:1; // 是否脏数据
    uint8_t reserved:1; // 保留位
    uint8_t timer:3; // 定时器
    uint8_t period:3; // 周期
} arp_map_t;

arp_map_t *get_arp_map();
void put_arp_map(ip_addr ip, mac_addr mac);
void flush_arp_map();
void clear_arp_map();
void kernel_setmac(ip_addr ip, mac_addr mac);

void arp_map_init();

#endif