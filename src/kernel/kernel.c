#include <x16/kernel.h>

void kernel_init()
{
    // 内存检测
    memory_check();
    BMB;
    // 开启A20地址线
    cli();
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x02);
    // 加载GDT
    init_gdt();
    BMB;
}