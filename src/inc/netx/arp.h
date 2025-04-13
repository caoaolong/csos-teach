#ifndef CSOS_ARP_H
#define CSOS_ARP_H

#include <types.h>

#define ARP_MAP_NR  51 // ARP映射数量

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

void arpl(arp_map_data_t *arp_data);

#endif