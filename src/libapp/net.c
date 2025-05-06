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