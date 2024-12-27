#include <csos/syscall.h>

void init_entry()
{
    while (TRUE)
    {
        sys_sleep(1000);
    }
}