#include <kernel.h>
#include <logf.h>
#include <task.h>

void default_entry()
{
    while (TRUE)
    {
        tty_logf("default task...");
        task_yield();
    }
}