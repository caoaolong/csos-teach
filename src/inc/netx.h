#ifndef CSOS_NETX_H
#define CSOS_NETX_H

#include <kernel.h>
#include <netx/eth.h>
#include <netx/arp.h>
#include <netx/ipv4.h>
#include <netx/arp_map.h>

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

uint16_t calc_checksum(uint8_t *data, uint32_t length) {
    uint32_t checksum = 0;

    // 确保数据长度是偶数
    length += length % 2;
    // 将数据分成 16 位的块
    for (uint32_t i = 0; i < length; i += 2) {
        uint16_t word = 0;
        if (i + 1 < length) {
            word = (data[i] << 8) + data[i + 1];
        } else {
            word = (data[i] << 8); // 如果是奇数长度，补零
        }
        checksum += word;
    }
    // 将高位和低位相加
    checksum = (checksum >> 16) + (checksum & 0xFFFF);
    // 取反
    return (uint16_t)~checksum;
}

uint32_t inet_pton(const char *ipstr);

void sys_arpl(arp_map_data_t *arp_data);
void sys_arpc();

#endif