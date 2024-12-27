#include <kernel.h>
#include <interrupt.h>
#include <csos/mutex.h>
#include <csos/syscall.h>

// 开始索引为3（0: unused, 1: kernel 32-code, 2: kernel 32-data, 3: user 32-code, 4: user 32-data, 5: syscall）
static uint32_t index = 6;
// GDT指针
static gdt_table_t *gdt;

extern mutex_t mutex;

void gdt32_init(gdt_table_t *gdt_table)
{
    gdt = gdt_table;
    set_syscall_gate(SYSCALL_GATE_SEG, 
        KERNEL_CODE_SEG, (uint32_t)syscall_handler, 
        GATE_ATTR_DPL3 | GATE_ATTR_P | GATE_TYPE_SYSCALL | SYSCALL_PMC);

    lgdt((uint32_t)gdt, sizeof(gdt_table_t) * GDT_SIZE);
}

uint32_t alloc_gdt_table_entry()
{
    mutex_lock(&mutex);
    for (int i = index; i < GDT_SIZE; i++)
    {
        if ((gdt + i)->attr == 0) {
            mutex_unlock(&mutex);
            return i << 3;
        }
    }
    mutex_unlock(&mutex);
    return -1;
}

void free_gdt_table_entry(uint32_t selector)
{
    mutex_lock(&mutex);
    gdt[selector >> 3].attr = 0;
    mutex_unlock(&mutex);
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

void set_syscall_gate(int selector, uint16_t segment, uint32_t offset, uint16_t attr) 
{
    gdt_gate_t *desc = (gdt_gate_t*)&gdt[selector >> 3];
	desc->offset_l = offset & 0xFFFF;
	desc->selector = segment;
	desc->attr = attr;
	desc->offset_h = (offset >> 16) & 0xFFFF;
}