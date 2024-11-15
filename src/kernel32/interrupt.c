#include <interrupt.h>
#include <kernel.h>
#include <os.h>

gate_t int_table[INTERRUPT_GATE_SIZE];

void set_interrupt_gate(int vector, uint32_t offset, uint32_t selector, uint16_t attr)
{
    gate_t *entry = &int_table[vector];
    entry->offset_l = offset & 0xFFFF;
    entry->selector = selector;
    entry->attr = attr;
    entry->offset_h = (offset >> 16) & 0xFFFF;
}

void interrupt_init()
{
    for (int i = 0; i < INTERRUPT_GATE_SIZE; i++)
    {
        set_interrupt_gate(i, (uint32_t)interrupt_handler_default, 
            KERNEL_CODE_SEG, 
            GATE_ATTR_P | GATE_ATTR_DPL0 | GAT_TYPE_INT);
    }

    lidt((uint32_t)int_table, INTERRUPT_GATE_SIZE);
}