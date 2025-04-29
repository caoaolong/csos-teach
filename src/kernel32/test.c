#include <kernel.h>
#include <logf.h>
#include <netx.h>
#include <netx/udp.h>
#include <task.h>
#include <pci/e1000.h>
#include <csos/string.h>

void sys_test()
{
    desc_buff_t *buff = alloc_desc_buff();
    netif_t *netif = netif_default();
    static char message[] = "Hello,World!";
    udp_build(netif, buff, "\xC0\xA8\x89\x01", 8000, 8000, message, sizeof(message));
}