#ifndef CSOS_PCI_H
#define CSOS_PCI_H

#include <kernel.h>

#define PCI_DEVICE_SIZE 10

#define PCI_CONF_ADDR   0xCF8
#define PCI_CONF_DATA   0xCFC
#define PCI_BAR_NR      6

#define PCI_CONF_VENDOR         0x0   // 厂商
#define PCI_CONF_DEVICE         0x2   // 设备
#define PCI_CONF_COMMAND        0x4   // 命令
#define PCI_CONF_STATUS         0x6   // 状态

typedef struct pci_classname_t {
    uint32_t classcode;
    const char *name;
} pci_classname_t;

typedef struct pci_device_t {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    uint16_t vendorid;
    uint16_t deviceid;
    uint8_t revision;
    uint32_t classcode;
} pci_device_t;

void pci_init();

#endif