#include <netx.h>
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

uint32_t inet_pton(const char *ipstr)
{
    uint32_t ip = 0;
    char *p = (char *)ipstr;
    while (*p != '\0') {
        if (*p == '.') {
            ip = (ip << 8) | (uint32_t)(*p - '0');
        } else if (*p >= '0' && *p <= '9') {
            ip = (ip << 8) | (uint32_t)(*p - '0');
        } else {
            return 0; // 无效的IP地址格式
        }
        p++;
    }
    return ip;
}