#include <kernel.h>
#include <tty.h>
#include <stdio.h>


void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    tty_printf("Hello, %d!", 3434);

    while(TRUE);
}