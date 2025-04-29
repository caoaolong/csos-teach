#include <os.h>
#include <task.h>
#include <logf.h>
#include <netx.h>
#include <netx/arp.h>
#include <interrupt.h>
#include <csos/string.h>

arp_map_t arp_map; // ARP映射表

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

void kernel_setmac(ip_addr ip, mac_addr mac)
{
    mac_addr gateway;
    // 1. 首先查询本地ARP缓存中是否有这个IP地址所对应的MAC地址
    BOOL found = FALSE;
    uint8_t try_count = 3;
    while ((!found) && try_count > 0) {
        for (int i = 0; i < arp_map.data.idx; i++) {
            if (!kernel_memcmp(arp_map.data.items[i].ip, ip, IPV4_LEN)) {
                kernel_memcpy(mac, arp_map.data.items[i].mac, MAC_LEN);
                found = TRUE;
                break;
            }
        }
        // 如果没有找到，则发送ARP请求
        if (!found) {
            logf("can't find the mac of %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            return;
        }
    }
}

void arp_map_init()
{
    // 500ms一次中断 2s刷新一次ARP缓存
    arp_map.timer = 0;
    arp_map.period = 4;
    arp_map.dirty = FALSE;
    read_disk(ARP_MAP_SECTOR, sizeof(arp_map.data) / SECTOR_SIZE, (uint16_t *)&arp_map.data);
}