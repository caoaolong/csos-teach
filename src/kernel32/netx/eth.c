#include <netx.h>
#include <interrupt.h>
#include <fs.h>
#include <kernel.h>
#include <pci/e1000.h>
#include <csos/string.h>

void eth_frame(e1000_t *e1000, desc_buff_t *buff, mac_addr target, uint16_t type)
{
    eth_t *eth = (eth_t *)buff->payload;
    kernel_memcpy(eth->dst, target, MAC_LEN);
    kernel_memcpy(eth->src, e1000->mac, MAC_LEN);
    eth->type = htons(type);
}