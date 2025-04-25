#include <kernel.h>
#include <logf.h>
#include <netx.h>
#include <task.h>
#include <pci/e1000.h>
#include <csos/string.h>

static char message[] = "Hello,World!";

void sys_test()
{
    ip_addr dst_ip;
    inet_pton("192.168.137.1", dst_ip);
    udp_send(dst_ip, 8000, 8000, message, sizeof(message));
    task_sleep(1000);
}