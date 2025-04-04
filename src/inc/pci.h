#ifndef CSOS_PCI_H
#define CSOS_PCI_H

#include <kernel.h>

#define PCI_CONF_ADDR   0xCF8
#define PCI_CONF_DATA   0xCFC
#define PCI_DEVICE_NR   10
#define PCI_BAR_NR      6

#define PCI_CONF_VENDOR         0x0   // 厂商
#define PCI_CONF_DEVICE         0x2   // 设备
#define PCI_CONF_COMMAND        0x4   // 命令
#define PCI_CONF_STATUS         0x6   // 状态
#define PCI_CONF_REVISION       0x8
#define PCI_CONF_BASE_ADDR0     0x10
#define PCI_CONF_BASE_ADDR1     0x14
#define PCI_CONF_BASE_ADDR2     0x18
#define PCI_CONF_BASE_ADDR3     0x1C
#define PCI_CONF_BASE_ADDR4     0x20
#define PCI_CONF_BASE_ADDR5     0x24
#define PCI_CONF_INTERRUPT      0x3C

#define PCI_BAR_IO_MASK         (~0x3)
#define PCI_BAR_MEM_MASK        (~0xF)

#define PCI_BAR_TYPE_MEM        0
#define PCI_BAR_TYPE_IO         1

#define PCI_COMMAND_MASTER      (1 << 2)  // Enable bus mastering

typedef struct pci_classname_t {
    uint32_t classcode;
    const char *name;
} pci_classname_t;

typedef struct pci_bar_t {
    uint8_t index:7;
    uint8_t type:1;
    uint32_t iobase;
    uint32_t size;
} pci_bar_t;

typedef struct pci_device_t {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    uint16_t vendorid;
    uint16_t deviceid;
    uint8_t revision;
    uint32_t classcode;
    pci_bar_t bar[PCI_BAR_NR];
} pci_device_t;

pci_device_t *pci_find_device(uint16_t vendorid, uint16_t deviceid);

void pci_enable_busmastering(pci_device_t *device);

void pci_set_bars(pci_device_t *device);

void pci_init();

#endif