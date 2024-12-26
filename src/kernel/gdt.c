__asm__(".code16gcc");
#include <kernel.h>

gdt_table_t gdt_table[GDT_SIZE] = {
    {0, 0, 0, 0}
};

void set_gdt_table_entry(int selector, uint32_t base, uint32_t limit, uint16_t attr)
{
    if (limit > 0xFFFFF) {
        attr |= 0x8000;
        limit >>= 12;
    }
    gdt_table_t *entry = &gdt_table[selector >> 3];
    entry->limit_l = limit & 0xFFFF;
    entry->base_l = base & 0xFFFF;
    entry->base_m = (base >> 16) & 0xFF;
    entry->attr = attr | (((limit >> 16) & 0xF) << 8);
    entry->base_h = (base >> 24) & 0xFF;
}

void init_gdt()
{
    for (int i = 0; i < GDT_SIZE; i++) {
        set_gdt_table_entry(i << 3, 0, 0, 0);
    }

    set_gdt_table_entry(KERNEL_DATA_SEG, 0, 0xFFFFFFFF, 
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_NORMAL | SEG_TYPE_DATA  | SEG_TYPE_RW | SEG_ATTR_D | SEG_ATTR_G);

    set_gdt_table_entry(KERNEL_CODE_SEG, 0, 0xFFFFFFFF, 
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_NORMAL | SEG_TYPE_CODE  | SEG_TYPE_RW | SEG_ATTR_D | SEG_ATTR_G);

    set_gdt_table_entry(USER_DATA_SEG, 0, 0xFFFFFFFF, 
        SEG_ATTR_P | SEG_ATTR_DPL3 | SEG_NORMAL | SEG_TYPE_DATA  | SEG_TYPE_RW | SEG_ATTR_D);
    set_gdt_table_entry(USER_CODE_SEG, 0, 0xFFFFFFFF, 
        SEG_ATTR_P | SEG_ATTR_DPL3 | SEG_NORMAL | SEG_TYPE_CODE  | SEG_TYPE_RW | SEG_ATTR_D);

    lgdt((uint32_t)gdt_table, (uint32_t)sizeof(gdt_table));
    show_string("GDT init success!\r\n");
}