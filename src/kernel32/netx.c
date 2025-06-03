#include <task.h>
#include <logf.h>
#include <paging.h>
#include <fs.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <netx/icmp.h>
#include <netx/tcp.h>
#include <netx/dhcp.h>
#include <pci/e1000.h>
#include <interrupt.h>
#include <csos/memory.h>
#include <csos/string.h>

#define PORT_SIZE 0x10000
static port_t *ports;

static netif_t netifs[4];
static socket_t sockets[1024];
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
        list_insert_back(&netif->wait_list, &buff->node);
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
    while (buff->refp == 0) {
        task_sleep(100);
    }
    desc_buff_t *rbuff = (desc_buff_t *)buff->refp;
    eth_t *eth = (eth_t *)rbuff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    icmp_echo_t *icmp = (icmp_echo_t *)ipv4->payload;
    logf("ICMPv4 Reply: from: %d.%d.%d.%d, length: %d, ttl: %d, data: %s",
        ipv4->src_ip[0], ipv4->src_ip[1], ipv4->src_ip[2], ipv4->src_ip[3],
        ntohs(ipv4->total_len), ipv4->ttl, (char *)icmp->payload);
}

void sys_ifconf(netif_dev_t *devs, int *devc)
{
    kernel_memset(devs, 0, sizeof(netif_dev_t) * 4);
    *devc = 0;
    for (int i = 0; i < 4; i++) {
        netif_t *netif = &netifs[i];
        if (netif->name[0] == 'e' && netif->name[1] == 'n') {
            kernel_memcpy(devs[*devc].name, netif->name, 8);
            kernel_memcpy(devs[*devc].ipv4, netif->ipv4, IPV4_LEN);
            kernel_memcpy(devs[*devc].gwv4, netif->gw, IPV4_LEN);
            kernel_memcpy(devs[*devc].mask, netif->mask, IPV4_LEN);
            kernel_memcpy(devs[*devc].mac, netif->mac, MAC_LEN);
            devs[*devc].status = netif->status;
            (*devc)++;
        }
    }
}

void sys_enum_port(port_t *port, uint16_t *cp, uint16_t *np)
{
    if (*cp == 0) {
        for (int i = 0; i < PORT_SIZE; i++) {
            port_t *pp = &ports[i];
            if (pp->status != PORT_DOWN) {
                kernel_memcpy(port, pp, sizeof(port_t));
                *cp = i;
                break;
            }
        }
    }
    *np = 0;
    for (int i = *cp + 1; i < PORT_SIZE; i++) {
        port_t *pp = &ports[i];
        if (pp->status != PORT_DOWN) {
            kernel_memcpy(port, pp, sizeof(port_t));
            *np = i;
            break;
        }
    }
}

int sys_socket(uint8_t family, uint8_t type, uint8_t flags)
{
    socket_t *socket = alloc_socket();
    if (!socket) {
        logf("No available socket");
        return -1; // 没有可用的socket
    }
    task_t *task = get_running_task();
    int fd = task_alloc_fd((FILE *)socket->fp);
    if (fd < 0) {
        free_socket(socket);
        logf("No available file descriptor");
        return -1; // 没有可用的文件描述符
    }
    socket->family = family;
    socket->type = type;
    socket->flags = flags;
    socket->seq = 0;
    socket->ack = 0;
    socket->dstp = socket->srcp = 0;
    socket->netif = netif_default();
    socket->state = TCP_CLOSED;
    ((FILE *)socket->fp)->sock = socket;
    return fd;
}

int sys_connect(int fd, sock_addr_t *addr, uint8_t addrlen)
{
    FILE *file = task_file(fd);
    socket_t *socket = file->sock;
    if (!socket) {
        logf("File descriptor %d is not a socket", fd);
        return -1;
    }
    socket->dstp = addr->port;
    kernel_memcpy(socket->dipv4, addr->ipv4, IPV4_LEN);
    if (socket->type == SOCK_DGRAM) {

    } else if (socket->type == SOCK_STREAM) {
        desc_buff_t *buff = alloc_desc_buff();
        tcp_syn(socket, buff, addr->ipv4, addr->port);
        // 默认为30s超时
        int tryc = 300;
        logf("Connecting to %d.%d.%d.%d:%d",
            addr->ipv4[0], addr->ipv4[1], addr->ipv4[2], addr->ipv4[3], ntohs(addr->port));
        while (socket->state != TCP_ESTABLISHED && tryc > 0) {
            task_sleep(100);
            tryc--;
        }
        logf("Connection established to %d.%d.%d.%d:%d",
            addr->ipv4[0], addr->ipv4[1], addr->ipv4[2], addr->ipv4[3], ntohs(addr->port));
        free_desc_buff(buff);
        return 0;
    }
    logf("unknown socket type(%d) for fd %d", socket->type, fd);
    return -1;
}

int sys_bind(int fd, sock_addr_t *addr, uint8_t addrlen)
{
    FILE *file = task_file(fd);
    socket_t *socket = file->sock;
    if (!socket) {
        logf("File descriptor %d is not a socket", fd);
        return -1;
    }
    socket->socktype = SOCK_SERVER;
    socket->srcp = addr->port;
    kernel_memcpy(socket->sipv4, netif_default()->ipv4, IPV4_LEN);
    if (socket->type == SOCK_DGRAM) {
        alloc_lazy_port(addr->port, socket, DBT_UDP);
    } else if (socket->type == SOCK_STREAM) {
        alloc_lazy_port(addr->port, socket, DBT_TCP);
    }
    return 0;
}

int sys_listen(int fd, int backlog)
{
    FILE *file = task_file(fd);
    socket_t *socket = file->sock;
    socket->backlog = backlog;
    if (!socket) {
        logf("File descriptor %d is not a socket", fd);
        return -1;
    }
    port_t *port = get_port(socket->srcp);
    port->status = PORT_LISTEN;
    socket->state = TCP_LISTEN;
    return 0;
}

int sys_accept(int fd, sock_addr_t *addr, uint8_t *addrlen)
{
    FILE *file = task_file(fd);
    socket_t *socket = file->sock;
    if (!socket) {
        logf("File descriptor %d is not a socket", fd);
        return -1;
    }
    port_t *port = get_port(socket->srcp);
    if (!port || port->status != PORT_LISTEN) {
        logf("Socket is not listening", fd);
        return -1;
    }
    logf("Listening on %d.%d.%d.%d:%d",
        addr->ipv4[0], addr->ipv4[1], addr->ipv4[2], addr->ipv4[3], socket->srcp);
    while (socket->state != TCP_ESTABLISHED) {
        task_sleep(100);
    }
    logf("client %d.%d.%d.%d:%d connected",
        socket->dipv4[0], socket->dipv4[1], socket->dipv4[2], socket->dipv4[3], socket->dstp);
    int cfd = fs_dup(fd);
    FILE *cfile = task_file(cfd);
    cfile->sock = alloc_socket();
    socket_t *csocket = cfile->sock;
    socket->state = TCP_ESTABLISHED;
    csocket->family = socket->family;
    csocket->type = socket->type;
    csocket->flags = socket->flags;
    csocket->srcp = socket->srcp;
    csocket->dstp = socket->dstp;
    kernel_memcpy(csocket->sipv4, socket->sipv4, IPV4_LEN);
    kernel_memcpy(csocket->dipv4, socket->dipv4, IPV4_LEN);
    csocket->seq = socket->seq;
    csocket->ack = socket->ack;
    return cfd;
}

int sys_close(int fd)
{
    FILE *file = task_file(fd);
    if (!file) {
        logf("File descriptor %d is not valid", fd);
        return -1;
    }
    socket_t *socket = file->sock;
    if (!socket) {
        logf("File descriptor %d is not a socket", fd);
        return -1;
    }
    desc_buff_t *buff = alloc_desc_buff();
    if (socket->socktype == SOCK_SERVER) {
        // TODO: 处理服务器端的关闭逻辑
    }
    tcp_finack(socket, buff, socket->dipv4);
    // 默认为3s超时
    int tryc = 30;
    logf("Disconnecting");
    while (socket->state != TCP_TIME_WAIT && tryc > 0) {
        task_sleep(100);
        tryc--;
    }
    // 等待两个MSL
    task_sleep(2 * 60);
    logf("Disconnected");
    task_free_fd(fd);
    free_socket(socket);
    free_desc_buff(buff);
    return 0;
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
    // kernel_memset(netifs, 0, sizeof(netifs));
    read_disk(NET_INFO_SECTOR, 1, (uint16_t *)netifs);
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
    // 端口初始化
    uint32_t psize = PORT_SIZE * sizeof(port_t) / PAGE_SIZE;
    ports = (port_t *)alloc_pages(psize);
    for (int i = 0; i < PORT_SIZE; i++) {
        port_t *p = &ports[i];
        p->status = PORT_DOWN;
        p->ptype = DBT_UNK;
        p->pid = 0;
        p->sock = NULL;
    }
    // 创建虚拟网卡
    // 本地回环接口: 127.0.0.1/8
    netif_create("\x7F\x00\x00\x01", "\xFF\x00\x00\x00", "\x00\x00\x00\x00", "\x00\x00\x00\x00\x00\x00");
    // 默认物理网卡: 0.0.0.0
    netif_create("\x00\x00\x00\x00", "\x00\x00\x00\x00", "\x00\x00\x00\x00", e1000->mac);
    // 初始化sockets
    kernel_memset(sockets, 0, sizeof(sockets));
}

void net_save()
{
    write_disk(NET_INFO_SECTOR, 1, (uint16_t *)netifs);    
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
    netif->period = 4;
    netif_count++;
    if (netif->status != NETIF_STATUS_ACK) {
        kernel_memset(netif, 0, sizeof(netif_t));
        kernel_strcpy(netif->name, netif_name);
        kernel_memcpy(netif->ipv4, ip, IPV4_LEN);
        kernel_memcpy(netif->mask, mask, IPV4_LEN);
        kernel_memcpy(netif->gw, gw, IPV4_LEN);
        kernel_memcpy(netif->mac, mac, MAC_LEN);
    }
    list_init(&netif->rx_list);
    list_init(&netif->tx_list);
    list_init(&netif->wait_list);

    // 协议栈初始化
    dhcp_init(netif);
    return 0;
}

uint16_t alloc_random_port(socket_t *socket, uint8_t protocol)
{
    uint16_t port = 20000;
    while (port < 65535) {
        if (ports[port].status == PORT_DOWN) {
            alloc_port(port, socket, protocol);
            break; // 找到一个空闲端口
        }
        port++;
    }
    return port;
}

static int alloc_system_port(uint16_t port, socket_t *socket, uint8_t protocol, uint8_t state)
{
    task_t *task = get_running_task();
    port_t *p = &ports[port];
    if (p->status != PORT_DOWN) {
        logf("Port %d is already in use", port);
        return -1; // 端口已被占用
    }
    p->status = state;
    p->pid = task->pid;
    p->ptype = protocol;
    p->sock = socket;
    return 0;
}

int alloc_port(uint16_t port, socket_t *socket, uint8_t protocol)
{
    return alloc_system_port(port, socket, protocol, PORT_UP);
}

int alloc_lazy_port(uint16_t port, socket_t *socket, uint8_t protocol)
{
    return alloc_system_port(port, socket, protocol, PORT_BUSY);
}

void free_port(uint16_t port)
{
    kernel_memset(&ports[port], 0, sizeof(port_t));
}

port_t *get_port(uint16_t port)
{
    if (port < PORT_SIZE) {
        return &ports[port];
    }
    return NULL; // 端口号无效
}

socket_t *alloc_socket()
{
    for (int i = 0; i < 1024; i++) {
        if (sockets[i].exists == 0) {
            kernel_memset(&sockets[i], 0, sizeof(socket_t));
            sockets[i].exists = 1;
            sockets[i].fp = alloc_page();
            sockets[i].socktype = SOCK_CLIENT;
            return &sockets[i];
        }
    }
    return NULL;
}

void free_socket(socket_t *socket)
{
    socket->state = TCP_CLOSED;
    socket->exists = 0;
    free_page(socket->fp);
}