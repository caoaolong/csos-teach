#ifndef CSOS_INTERRUPT_H
#define CSOS_INTERRUPT_H

#include <types.h>

// 中断向量及处理函数
extern void interrupt_handler_default();

#define INTERRUPT_GATE_SIZE 0x100

#define GAT_TYPE_INT    (0xE << 8)
#define GATE_ATTR_P     (1 << 15)
#define GATE_ATTR_DPL0  (0 << 13)
#define GATE_ATTR_DPL3  (3 << 13)

typedef struct gate_t
{
    uint16_t offset_l;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset_h;
} gate_t;

void set_interrupt_gate(int vector, uint32_t offset, uint32_t selector, uint16_t attr);

void interrupt_init();

#endif