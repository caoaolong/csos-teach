#include <task.h>
#include <logf.h>
#include <paging.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <netx/icmp.h>
#include <netx/dhcp.h>
#include <csos/memory.h>
#include <csos/string.h>

static netif_t netifs[4];
static uint32_t netif_count = 0;
static sem_t netin_sem, netout_sem;
static task_t netin_task, netout_task;

netif_t *netif_default()
{
    return &netifs[1];
}

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
                list_node_t *first = list_remove_front(&netif->rx_list);
                desc_buff_t *buff = struct_from_field(first, desc_buff_t, node);
                logf("%s recv packet: %d", netif->name, buff->length);
                eth_input(netif, buff);
                count++;
            }
        }
        if (count == 0) {
            logf("netin thread is waiting...");
            sem_wait(&netin_sem);
        }
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
                list_node_t *first = list_remove_front(&netif->tx_list);
                desc_buff_t *buff = struct_from_field(first, desc_buff_t, node);
                logf("%s send packet: %d", netif->name, buff->length);
                e1000_send_packet(buff);
                count++;
            }
        }
        if (count == 0) {
            logf("netout thread is waiting...");
            sem_wait(&netout_sem);
        }
    }
}

static netif_t *find_netif(mac_addr mac)
{
    // 0为本地回环接口
    // 1为默认物理网卡接口
    if (!kernel_memcmp(mac, "\xFF\xFF\xFF\xFF\xFF\xFF", MAC_LEN))
        return &netifs[1];
    for (int i = 0; i < netif_count; i++) {
        netif_t *netif = &netifs[i];
        if (!kernel_memcmp(netif->mac, mac, MAC_LEN)) {
            return netif;
        }
    }
    return NULL;
}

uint16_t calc_checksum(uint8_t *data, uint32_t length)
{
    uint32_t checksum = 0;

    for (uint32_t i = 0; i < length; i += 2) {
        uint16_t word = data[i] << 8;
        if (i + 1 < length) {
            word |= data[i + 1];
        }
        checksum += word;
    }

    // 不断将高 16 位加到低 16 位
    while (checksum >> 16) {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }

    return htons((uint16_t)(~checksum));
}

// 接收数据包
void netif_input(desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    netif_t *netif = find_netif(eth->dst);
    if (netif) {
        list_insert_back(&netif->rx_list, &buff->node);
        sem_notify(&netin_sem);
    } else {
        free_desc_buff(buff);
    }
}

// 发送数据包
void netif_output(desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    netif_t *netif = find_netif(eth->src);
    if (netif) {
        list_insert_back(&netif->tx_list, &buff->node);
        sem_notify(&netout_sem);
    } else {
        free_desc_buff(buff);
    }
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
    netif_t *netif = netif_default();
    desc_buff_t *buff = alloc_desc_buff();
    icmp_build(netif, buff, ICMP_TYPE_ECHO_REQUEST, 0, dst_ip, NULL, 0);
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

void net_init()
{
    // 网卡初始化
    e1000_t *e1000 = e1000_init();
    // 初始化虚拟网卡列表
    kernel_memset(netifs, 0, sizeof(netifs));
    int ret;
    // 创建数据收发线程
    uint32_t netin_stack = alloc_page();
    sem_init(&netin_sem, 0);
    ret = task_init(&netin_task, "netifin", TASK_LEVEL_SYSTEM, (uint32_t)netin_thread, netin_stack - PAGE_SIZE);
    if (ret < 0) {
        logf("netifin task create failed");
        free_page(netin_stack);
        return;
    }
    uint32_t netout_stack = alloc_page();
    sem_init(&netout_sem, 0);
    ret = task_init(&netout_task, "netifout", TASK_LEVEL_SYSTEM, (uint32_t)netout_thread, netout_stack - PAGE_SIZE);
    if (ret < 0) {
        logf("netifout task create failed");
        free_page(netout_stack);
        return;
    }
    // 创建虚拟网卡
    // 本地回环接口: 127.0.0.1/8
    netif_create("\x7F\x00\x00\x01", "\xFF\x00\x00\x00", "\x00\x00\x00\x00", "\x00\x00\x00\x00\x00\x00");
    // 默认物理网卡: 192.168.137.100(临时IP)
    netif_create("\x00\x00\x00\x00", "\x00\x00\x00\x00", "\x00\x00\x00\x00", e1000->mac);
    // 获取IP地址
    dhcp_discover(netif_default(), alloc_desc_buff());
}

static char netif_name[] = "en?";

int netif_create(ip_addr ip, ip_addr mask, ip_addr gw, mac_addr mac)
{
    if (netif_count >= 4) {
        logf("The number of netif exceeds the upper limit");
        return -1; // 超过最大网卡数量
    }
    netif_name[2] = '0' + netif_count;
    netif_t *netif = &netifs[netif_count];
    netif->index = netif_count;
    netif_count++;
    kernel_memset(netif, 0, sizeof(netif_t));
    kernel_strcpy(netif->name, netif_name);
    kernel_memcpy(netif->ipv4, ip, IPV4_LEN);
    kernel_memcpy(netif->mask, mask, IPV4_LEN);
    kernel_memcpy(netif->gw, gw, IPV4_LEN);
    kernel_memcpy(netif->mac, mac, MAC_LEN);
    list_init(&netif->rx_list);
    list_init(&netif->tx_list);
    return 0;
}