#include <netx.h>
#include <task.h>
#include <logf.h>
#include <csos/memory.h>
#include <csos/string.h>

static netif_t netifs[4];
static uint32_t netif_count = 0;
static sem_t netin_sem, netout_sem;
static task_t netin_task, netout_task;

// 数据接收线程
static void netin_thread()
{
    while (TRUE) {
        int count = 0;
        for (int i = 0; i < netif_count; i++) {
            netif_t *netif = &netifs[i];
            if (list_is_empty(&netif->rx_list)) {
                continue;
            } else {
                // TODO: 处理接收数据包
                count++;
            }
        }
        if (count == 0) sem_wait(&netin_sem);
    }
}

// 数据发送线程
static void netout_thread()
{
    while (TRUE) {
        int count = 0;
        for (int i = 0; i < netif_count; i++) {
            netif_t *netif = &netifs[i];
            if (list_is_empty(&netif->tx_list)) {
                continue;
            } else {
                // TODO: 处理接收数据包
                count++;
            }
        }
        if (count == 0) sem_wait(&netout_sem);
    }
}

void netif_input(netif_t *netif, desc_buff_t *buff)
{
    list_insert_back(&netif->rx_list, buff);
    sem_notify(&netif->rx_sem);
}

void netif_output(netif_t *netif, desc_buff_t *buff)
{
    list_insert_back(&netif->tx_list, buff);
    sem_notify(&netif->tx_sem);
}

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
    e1000_t *e1000 = get_e1000dev();
    if (!kernel_memcmp(dst_ip, e1000->ipv4, IPV4_LEN)) {
        kernel_memcpy(dst_mac, e1000->mac, MAC_LEN);
    } else {
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

void net_init()
{
    // 网卡初始化
    e1000_init();
    e1000_t *e1000 = get_e1000dev();
    // 设置自己的临时IP(192.168.137.100)
    kernel_memcpy(e1000->ipv4, "\xC0\xA8\x89\x64", IPV4_LEN);
    // 初始化虚拟网卡列表
    kernel_memset(netifs, 0, sizeof(netifs));
    // 创建数据收发线程
    uint32_t netin_stack = alloc_page();
    sem_init(&netin_sem, 0);
    task_init(&netin_task, "netin", TASK_LEVEL_SYSTEM, (uint32_t)netin_task, netin_stack);
    uint32_t netout_stack = alloc_page();
    sem_init(&netout_sem, 0);
    task_init(&netout_task, "netout", TASK_LEVEL_SYSTEM, (uint32_t)netout_task, netout_stack);
}

int netif_create(ip_addr ip, ip_addr mask, ip_addr gw)
{
    if (netif_count >= 4) {
        logf("The number of netif exceeds the upper limit");
        return -1; // 超过最大网卡数量
    }
    netif_t *netif = &netifs[netif_count++];
    kernel_memcpy(netif->ipv4, ip, IPV4_LEN);
    kernel_memcpy(netif->mask, mask, IPV4_LEN);
    kernel_memcpy(netif->gw, gw, IPV4_LEN);
    kernel_memset(netif, 0, sizeof(netif_t));
    list_init(&netif->rx_list);
    list_init(&netif->tx_list);
    return 0;
}