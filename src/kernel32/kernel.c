#include <kernel.h>
#include <tty.h>
#include <stdio.h>
#include <interrupt.h>

void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    interrupt_init();
    int a = 3 / 0;
    while(TRUE);
}