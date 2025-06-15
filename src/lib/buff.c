#include <paging.h>
#include <logf.h>
#include <pci/e1000.h>
#include <netx/eth.h>
#include <netx/arp.h>
#include <netx/ipv4.h>
#include <netx/icmp.h>
#include <netx/tcp.h>
#include <csos/mutex.h>
#include <csos/memory.h>
#include <csos/string.h>

static list_t buff_list;
static mutex_t mutex;

static void reply_arp_desc_buff(netif_t *netif, desc_buff_t *rxbuff)
{
    eth_t *rxeth = (eth_t *)rxbuff->payload;
    arp_t *rxarp = (arp_t *)rxeth->payload;

    list_node_t *pnode = list_get_first(&netif->wait_list);
    while (pnode) {
        desc_buff_t *txbuff = struct_from_field(pnode, desc_buff_t, node);
        arp_t *txarp = (arp_t *)((eth_t *)txbuff->payload)->payload;
        if (!kernel_memcmp(txarp->dst_ip, rxarp->src_ip, IPV4_LEN)) {
            txbuff->refp = (uint32_t)rxbuff;
            list_remove(&netif->wait_list, pnode);
            return;
        }
        pnode = list_get_next(pnode);
    }
}

static void reply_icmp_desc_buff(netif_t *netif, desc_buff_t *rxbuff)
{
    eth_t *rxeth = (eth_t *)rxbuff->payload;
    ipv4_t *rxipv4 = (ipv4_t *)rxeth->payload;
    icmp_echo_t *rxicmp = (icmp_echo_t *)rxipv4->payload;

    list_node_t *pnode = list_get_first(&netif->wait_list);
    while (pnode) {
        desc_buff_t *txbuff = struct_from_field(pnode, desc_buff_t, node);
        ipv4_t *txipv4 = (ipv4_t *)((eth_t *)txbuff->payload)->payload;
        icmp_echo_t *txicmp = (icmp_echo_t *)txipv4->payload;
        if (rxicmp->id == txicmp->id && !kernel_memcmp(txipv4->dst_ip, rxipv4->src_ip, IPV4_LEN)) {
            txbuff->refp = (uint32_t)rxbuff;
            list_remove(&netif->wait_list, pnode);
            break;
        }
        pnode = list_get_next(pnode);
    }
}

void reply_desc_buff(netif_t *netif, desc_buff_t *buff, uint8_t type)
{
    switch (type)
    {
    case DBT_ARP:
        reply_arp_desc_buff(netif, buff);
        break;
    case DBT_ICMP:
        reply_icmp_desc_buff(netif, buff);
        break;
    default:
        free_desc_buff(buff);
        break;
    }
}

void free_desc_buff(desc_buff_t *buff)
{
    mutex_lock(&mutex);
    list_t *list = &buff_list;
    list_node_t *node = list_remove(list, &buff->node);
    if (list->size % 2 == 0) {
        free_page((uint32_t)node & ~0xFFF);
    }
    mutex_unlock(&mutex);
}

desc_buff_t *alloc_desc_buff()
{
    mutex_lock(&mutex);
    list_t *list = &buff_list;
    desc_buff_t *buf = NULL;
    if (list->size % 2 == 0) {
        buf = (desc_buff_t *)alloc_page();
    } else {
        list_node_t *last = list_get_last(list);
        uint8_t *pbuf = (uint8_t *)struct_from_field(last, desc_buff_t, node);
        buf = (desc_buff_t *)(pbuf + (PAGE_SIZE / 2));
    }
    buf->length = 0;
    buf->refp = 0;
    list_node_init(&buf->node);
    list_insert_back(list, &buf->node);
    mutex_unlock(&mutex);
    return buf;
}

void desc_buff_init()
{
    list_init(&buff_list);
    mutex_init(&mutex);
}