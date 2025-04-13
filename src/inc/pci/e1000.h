#ifndef CSOS_E1000_H
#define CSOS_E1000_H

#include <kernel.h>
#include <task.h>
#include <pci.h>
#include <list.h>
#include <netx.h>

#define NET_DEV_NAME_LEN    16

typedef struct desc_buff_t
{
	list_node_t node;
	uint16_t length;
	uint16_t refc;
	uint8_t payload[0];
} desc_buff_t;

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
    tx_desc_t *tx;
    uint16_t tx_now;

	task_t *tx_waiter;
	list_t desc_list;
} e1000_t;

void free_desc_buff(e1000_t *dev, desc_buff_t *buff);
desc_buff_t *alloc_desc_buff(e1000_t *dev);

void test_send_packet();

void e1000_init();

#endif