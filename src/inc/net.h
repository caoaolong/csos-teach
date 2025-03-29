#ifndef CSOS_NET_H
#define CSOS_NET_H

#include <types.h>
#include <pci.h>

#define NET_DEV_NAME_LEN    16

// 接收描述符
typedef struct rx_desc_t
{
    uint64_t addr;     // 地址
    uint16_t length;   // 长度
    uint16_t checksum; // 校验和
    uint8_t status;    // 状态
    uint8_t error;     // 错误
    uint16_t special;  // 特殊
} rx_desc_t;

// 传输描述符
typedef struct tx_desc_t
{
    uint64_t addr;    // 缓冲区地址
    uint16_t length;  // 包长度
    uint8_t cso;      // Checksum Offset
    uint8_t cmd;      // 命令
    uint8_t status;   // 状态
    uint8_t css;      // Checksum Start Field
    uint16_t special; // 特殊
} tx_desc_t;

typedef struct e1000_t {
    char name[NET_DEV_NAME_LEN];
    uint32_t base;
    mac_addr mac;
    pci_device_t *dev;
    uint8_t eeprom;

    rx_desc_t *rx_desc; // 接收描述符
    uint16_t rx_cur;    // 接收描述符指针
    tx_desc_t *tx_desc; // 传输描述符
    uint16_t tx_cur;    // 传输描述符指针
} e1000_t;

void net_init();

#endif