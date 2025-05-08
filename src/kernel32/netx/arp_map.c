#include <os.h>
#include <task.h>
#include <logf.h>
#include <netx.h>
#include <netx/arp.h>
#include <interrupt.h>
#include <csos/string.h>

arp_map_t arp_map; // ARP映射表

static BOOL find_mac(ip_addr ip, mac_addr mac)
{
    for (int i = 0; i < arp_map.data.idx; i++) {
        if (!kernel_memcmp(arp_map.data.items[i].ip, ip, IPV4_LEN)) {
            kernel_memcpy(mac, arp_map.data.items[i].mac, MAC_LEN);
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL ask_mac(netif_t *netif, ip_addr ip, mac_addr mac, uint8_t tryc)
{
    while (tryc--) {
        desc_buff_t *buff = alloc_desc_buff();
        arp_build(netif, buff, ip);
        while (buff->refp == 0) {
            task_sleep(100);
        }
        desc_buff_t *rbuff = (desc_buff_t *)buff->refp;
        eth_t *eth = (eth_t *)rbuff->payload;
        arp_t *arp = (arp_t *)eth->payload;
        kernel_memcpy(mac, arp->src_mac, MAC_LEN);
        return TRUE;
    }
    return FALSE;
}

arp_map_t *get_arp_map()
{
    return &arp_map;
}

void clear_arp_map()
{
    kernel_memset(&arp_map.data, 0, sizeof(arp_map.data));
    arp_map.dirty = TRUE;
}

void put_arp_map(ip_addr ip, mac_addr mac)
{
    if (arp_map.data.idx >= ARP_MAP_NR)
        return;
    int length = arp_map.data.idx;
    for (int i = 0; i < length; i++) {
        arp_map_item_t *item = &arp_map.data.items[i];
        if (!kernel_memcmp(item->mac, mac, MAC_LEN)) {
            kernel_memcpy(item->ip, ip, IPV4_LEN);
            return;
        }
    }
    arp_map_item_t *item = &arp_map.data.items[arp_map.data.idx];
    kernel_memcpy(item->ip, ip, IPV4_LEN);
    kernel_memcpy(item->mac, mac, MAC_LEN);
    arp_map.dirty = TRUE;
    arp_map.data.idx++;
}

void flush_arp_map()
{
    if (!arp_map.dirty) return;

    protect_state_t ps = protect_enter();
    uint32_t amsize = sizeof(arp_map.data);
    write_disk(ARP_MAP_SECTOR, amsize / SECTOR_SIZE, (uint16_t *)&arp_map.data);
    arp_map.dirty = FALSE;
    protect_exit(ps);
}

BOOL kernel_setmac(netif_t *netif, ip_addr ip, mac_addr mac)
{
    // 如果是广播则直接返回广播的MAC地址
    if (!kernel_memcmp(ip, "\xFF\xFF\xFF\xFF", IPV4_LEN)) {
        kernel_memcpy(mac, "\xFF\xFF\xFF\xFF\xFF\xFF", MAC_LEN);
        return TRUE;
    }
    // 判断目标IP地址是否处于同一子网内
    uint32_t mask = ip2uint32(netif->mask);
    uint32_t ipm = ip2uint32(ip) & mask;
    uint32_t selfm = ip2uint32(netif->ipv4) & mask;
    if (ipm == selfm) {
        // 查找目标主机的MAC地址
        if (find_mac(ip, mac)) {
            return TRUE;
        }
        // 请求目标主机的MAC地址
        if (ask_mac(netif, ip, mac, 3)) {
            return TRUE;
        }
    } else {
        // 查找网关的MAC地址
        if (find_mac(netif->gw, mac)) {
            return TRUE;
        }
        // 请求网关的MAC地址
        if (ask_mac(netif, ip, mac, 3)) {
            return TRUE;
        }
    }
    logf("can't find the mac of %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return FALSE;
}

void arp_map_init()
{
    // 500ms一次中断 2s刷新一次ARP缓存
    arp_map.timer = 0;
    arp_map.period = 4;
    arp_map.dirty = FALSE;
    read_disk(ARP_MAP_SECTOR, sizeof(arp_map.data) / SECTOR_SIZE, (uint16_t *)&arp_map.data);
}