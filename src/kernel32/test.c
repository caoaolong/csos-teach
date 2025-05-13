#include <kernel.h>
#include <logf.h>
#include <netx.h>
#include <netx/tcp.h>

void sys_test()
{
    desc_buff_t *buff = alloc_desc_buff();
    netif_t *netif = netif_default();
    tcp_syn(netif, buff, "\xC0\xA8\x89\x01", 8000, 8000);
}