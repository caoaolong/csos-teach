#include <csos/syscall.h>
#include <netx/inet.h>

void arpl(arp_map_data_t *arp_data)
{
    syscall_arg_t args = {SYS_NR_ARPL, (int)arp_data, 0, 0, 0};
    _syscall(&args);
}

void arpc()
{
    syscall_arg_t args = {SYS_NR_ARPC, 0, 0, 0, 0};
    _syscall(&args);
}

void ping(const char *ip)
{
    syscall_arg_t args = {SYS_NR_PING, (int)ip, 0, 0, 0};
    _syscall(&args);
}

void ifconf(netif_dev_t *devs, int *devc)
{
    syscall_arg_t args = {SYS_NR_IFCONF, (int)devs, (int)devc, 0, 0};
    _syscall(&args);
}

void enum_port(port_t *port, uint16_t *cp, uint16_t *np)
{
    syscall_arg_t args = {SYS_NR_ENUM_PORT, (int)port, (int)cp, (int)np, 0};
    _syscall(&args);
}

int socket(uint16_t family, uint8_t type, uint8_t flags)
{
    syscall_arg_t args = {SYS_NR_SOCKET, family, type, flags, 0};
    return _syscall(&args);
}

int connect(int fd, sock_addr_t addr, uint8_t addrlen)
{
    syscall_arg_t args = {SYS_NR_CONNECT, fd, (int)&addr, addrlen, 0};
    return _syscall(&args);
}

int close(int fd)
{
    syscall_arg_t args = {SYS_NR_CLOSE, fd, 0, 0, 0};
    return _syscall(&args);
}

void inet_pton(const char *ipstr, ip_addr ipv)
{
    char *p = (char *)ipstr;
    int idx = 0;
    memset(ipv, 0, IPV4_LEN);
    while (*p) {
        if (*p == '.') {
            idx++;
        } else {
            ipv[idx] = ipv[idx] * 10 + (*p - '0');
        }
        p++;
    }
}