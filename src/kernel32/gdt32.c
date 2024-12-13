#include <kernel.h>
#include <interrupt.h>

// 开始索引为3（0: unused, 1: 32-code, 2: 32-data）
static uint32_t index = 3;
// GDT指针
static gdt_table_t *gdt;

void gdt32_init(gdt_table_t *gdt_table)
{
    gdt = gdt_table;
}

uint32_t alloc_gdt_table_entry()
{
    protect_state_t ps = protect_enter();
    for (int i = index; i < GDT_SIZE; i++)
    {
        if ((gdt + i)->attr == 0) {
            protect_exit(ps);
            return i << 3;
        }
    }
    protect_exit(ps);
    return -1;
}

void free_gdt_table_entry(uint32_t selector)
{
    gdt[selector >> 3].attr = 0;
}

void set_gdt_table_entry(int selector, uint32_t base, uint32_t limit, uint16_t attr)
{
    if (limit > 0xFFFFF) {
        attr |= 0x8000;
        limit /= 0x1000;
    }
    gdt_table_t *entry = &gdt[selector >> 3];
    entry->limit_l = limit & 0xFFFF;
    entry->base_l = base & 0xFFFF;
    entry->base_m = (base >> 16) & 0xFF;
    entry->attr = attr | (((limit >> 16) & 0xF) << 8);
    entry->base_h = (base >> 24) & 0xFF;
}