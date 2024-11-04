__asm__(".code16gcc");
#include <kernel.h>

void kernel_init()
{
    // 内存检测
    memory_check();
    // 开启A20地址线
    cli();
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x02);
    // 加载GDT
    init_gdt();
    // 开启保护模式(cr0第0位置为1)
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));
    // 跳转
    far_jump(KERNEL_CODE_SEG, (uint32_t)protect_mode);
}