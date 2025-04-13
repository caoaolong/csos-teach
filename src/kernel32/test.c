#include <kernel.h>
#include <logf.h>
#include <netx.h>
#include <pci/e1000.h>

void sys_test()
{
    // test_send_packet();
    put_arp_map("\x12\x12\x12\x12", "\x45\x67\x89\xAB\xCD\xEF");
}