#include <kernel.h>
#include <tty.h>
#include <stdio.h>


void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    tty_printf("Hello,World,%d!\nHello,%s\n", 13, "CSOS");
    while(TRUE);
}