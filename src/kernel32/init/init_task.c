#include <kernel.h>
#include <logf.h>
#include <task.h>

void init_entry()
{
    while (TRUE)
    {
        tty_logf("init task...");
        task_yield();
    }
}