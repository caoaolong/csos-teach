#include <kernel.h>
#include <logf.h>
#include <netx.h>
#include <task.h>
#include <netx/tcp.h>
#include <csos/string.h>

void sys_test()
{
    desc_buff_t *buff = alloc_desc_buff();
    netif_t *netif = netif_default();
    tcp_syn(netif, buff, "\xC0\xA8\x89\x01");
    logf("connected to server");
    task_sleep(1000);
    kernel_memset(buff, 0, sizeof(desc_buff_t));
    tcp_finack(netif, buff, "\xC0\xA8\x89\x01");
}