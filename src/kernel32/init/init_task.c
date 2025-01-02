#include <csos/syscall.h>

void init_entry()
{
    uint32_t counter = 0;
    while (TRUE)
    {
        sys_logf("init task : %d", sys_getpid());
        sys_sleep(1000);
    }
}