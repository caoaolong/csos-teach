#ifndef CSOS_E1000_H
#define CSOS_E1000_H

#include <kernel.h>
#include <task.h>
#include <pci.h>
#include <list.h>
#include <netx.h>

#define NET_DEV_NAME_LEN    16

// RX descriptor structure
typedef struct rx_desc_t
{
	volatile uint64_t	address;
	volatile uint16_t	length;
	volatile uint16_t	checksum;
	volatile uint8_t	status;
	volatile uint8_t	errors;
	volatile uint16_t	special;
} rx_desc_t;

// TX descriptor structure
typedef struct tx_desc_t
{
	volatile uint64_t	address;
	volatile uint16_t	length;
	volatile uint8_t	cso;
	volatile uint8_t	cmd;
	volatile uint8_t	sta;
	volatile uint8_t	css;
	volatile uint16_t	special;
} tx_desc_t;

typedef struct e1000_t {
    char name[NET_DEV_NAME_LEN];
    uint32_t base;
    mac_addr mac;
    pci_device_t *dev;
    uint8_t eeprom;

    rx_desc_t *rx;
    uint16_t rx_now;
	desc_buff_t **rx_buff;

	tx_desc_t *tx;
    uint16_t tx_now;
	desc_buff_t **tx_buff;

	task_t *tx_waiter;
} e1000_t;

void e1000_send_packet(desc_buff_t *buff);
e1000_t *e1000_init();

#endif