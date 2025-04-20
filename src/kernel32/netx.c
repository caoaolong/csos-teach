#include <netx.h>
#include <task.h>
#include <csos/string.h>

void sys_arpl(arp_map_data_t *arp_data)
{
    arp_map_t *arp_map = get_arp_map();
    kernel_memcpy(arp_data, &arp_map->data, sizeof(arp_map_data_t));
}

void sys_arpc()
{
    clear_arp_map();
    flush_arp_map();
}

void sys_ping(const char *ip)
{
    ip_addr dst_ip;
    mac_addr dst_mac;
    inet_pton(ip, dst_ip);
    // 1. 首先查询本地ARP缓存中是否有这个IP地址所对应的MAC地址
    BOOL found = FALSE;
    arp_map_t *arp_map;
    while (!found) {
        arp_map = get_arp_map();
        for (int i = 0; i < arp_map->data.idx; i++) {
            if (!kernel_memcmp(arp_map->data.items[i].ip, dst_ip, IPV4_LEN)) {
                kernel_memcpy(dst_mac, arp_map->data.items[i].mac, MAC_LEN);
                found = TRUE;
                break;
            }
        }
        // 如果没有找到，则发送ARP请求
        if (!found) {
            arp_send(dst_ip);
            task_sleep(100); // 等待100ms
        }
    }
    // 2. 如果找到了MAC地址，则发送ICMP请求
    icmp_send(dst_mac, dst_ip);
}

void inet_pton(const char *ipstr, ip_addr ipv)
{
    char *p = (char *)ipstr;
    int idx = 0;
    kernel_memset(ipv, 0, IPV4_LEN);
    while (*p) {
        if (*p == '.') {
            idx++;
        } else {
            ipv[idx] = ipv[idx] * 10 + (*p - '0');
        }
        p++;
    }
}

uint16_t calc_checksum(uint8_t *data, uint32_t length) {
    uint32_t checksum = 0;

    // 确保数据长度是偶数
    length += length % 2;
    // 将数据分成 16 位的块
    for (uint32_t i = 0; i < length; i += 2) {
        uint16_t word = 0;
        if (i + 1 < length) {
            word = (data[i] << 8) + data[i + 1];
        } else {
            word = (data[i] << 8); // 如果是奇数长度，补零
        }
        checksum += word;
    }
    // 将高位和低位相加
    checksum = (checksum >> 16) + (checksum & 0xFFFF);
    // 取反
    return (uint16_t)htons(~checksum);
}