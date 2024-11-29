#include <kernel.h>
#include <tty.h>
#include <interrupt.h>
#include <logf.h>

void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    interrupt_init();
    tty_logf_init();
    tty_logf("KL Version: %s; OS Version: %s", KERNEL_VERSION, OP_SYS_VERSION);
    sti();
    while(TRUE);
}