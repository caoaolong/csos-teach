#include <csos/syscall.h>
#include <netx/arp.h>

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