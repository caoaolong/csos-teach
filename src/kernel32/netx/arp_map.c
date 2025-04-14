#include <os.h>
#include <netx/arp_map.h>
#include <interrupt.h>

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

void arp_map_init()
{
    // 500ms一次中断 2s刷新一次ARP缓存
    arp_map.timer = 0;
    arp_map.period = 4;
    arp_map.dirty = FALSE;
    read_disk(ARP_MAP_SECTOR, sizeof(arp_map.data) / SECTOR_SIZE, (uint16_t *)&arp_map.data);
}