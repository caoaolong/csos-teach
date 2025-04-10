#ifndef CSOS_NETX_H
#define CSOS_NETX_H

#include <kernel.h>
#include <netx/eth.h>

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

#endif