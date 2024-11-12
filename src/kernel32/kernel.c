#include <kernel.h>
#include <tty.h>


void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    char *str = "Hello,World!";
    tty_write(str, 12);

    while(TRUE);
}