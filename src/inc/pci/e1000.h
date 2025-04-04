#ifndef CSOS_E1000_H
#define CSOS_E1000_H

#include <kernel.h>
#include <pci.h>

#define NET_DEV_NAME_LEN    16

typedef struct e1000_t {
    char name[NET_DEV_NAME_LEN];
    uint32_t base;
    mac_addr mac;
    pci_device_t *dev;
    uint8_t eeprom;
} e1000_t;

void e1000_init();

#endif