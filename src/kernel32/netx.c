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
    return (uint16_t)~checksum;
}