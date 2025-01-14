#include <kernel.h>
#include <paging.h>

#define OS_ADDR     0x100000

#define CR4_PSE (1 << 4)
#define CR0_PG  (1 << 31)
#define PDE_P   (1 << 0)
#define PDE_W   (1 << 1)
#define PDE_PS  (1 << 7)

extern memory_info_t memory_info;
extern gdt_table_t gdt_table;

void enable_paging()
{
    static uint32_t page_dir[1024] __attribute__((aligned(PAGE_SIZE))) = {
        [0] = PDE_P | PDE_W | PDE_PS | 0
    };
    write_cr4(read_cr4() | CR4_PSE);
    write_cr3((uint32_t)page_dir);
    write_cr0(read_cr0() | CR0_PG);
}

void kernel32_init()
{
    // 0号扇区: 引导扇区
    // 1-64: Kernel x16
    // 65-*: Kernel x86
    read_disk(65, 500, (uint16_t*)OS_ADDR);
    uint32_t addr = read_elf_header((uint8_t*)OS_ADDR);
    if (addr == 0)
        while (TRUE);
    // 开启分页
    enable_paging();
    ((void (*)(memory_info_t*, uint32_t))addr)(&memory_info, (uint32_t)&gdt_table);
}