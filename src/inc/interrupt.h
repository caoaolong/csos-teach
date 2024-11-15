#ifndef CSOS_INTERRUPT_H
#define CSOS_INTERRUPT_H

#include <types.h>

typedef struct interrupt_frame_t
{
    // 手动压入
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    // 自动压入
    uint32_t eip, cs, eflags;
} interrupt_frame_t;

// 中断向量及处理函数
extern void interrupt_handler_default();
void handler_default(interrupt_frame_t frame);
#define IRQ0_DE             0
extern void interrupt_handler_division();
void handler_division(interrupt_frame_t frame);
#define IRQ1_DB             1
extern void interrupt_handler_debug();
void handler_debug(interrupt_frame_t frame);
#define IRQ2_NMI            2
extern void interrupt_handler_nmi();
void handler_nmi(interrupt_frame_t frame);
#define IRQ3_BP             3
extern void interrupt_handler_breakpoint();
void handler_breakpoint(interrupt_frame_t frame);
#define IRQ4_OF             4
extern void interrupt_handler_overflow();
void handler_overflow(interrupt_frame_t frame);
#define IRQ5_BR             5
extern void interrupt_handler_range();
void handler_range(interrupt_frame_t frame);
#define IRQ6_UD             6
extern void interrupt_handler_opcode();
void handler_opcode(interrupt_frame_t frame);
#define IRQ7_NM             7
extern void interrupt_handler_device();
void handler_device(interrupt_frame_t frame);
#define IRQ8_DF             8
extern void interrupt_handler_double();
void handler_double(interrupt_frame_t frame);
#define IRQ9_CSO            9

#define IRQA_TS             10
extern void interrupt_handler_tss();
void handler_tss(interrupt_frame_t frame);
#define IRQB_NP             11
extern void interrupt_handler_segment();
void handler_segment(interrupt_frame_t frame);
#define IRQC_SS             12
extern void interrupt_handler_stack();
void handler_stack(interrupt_frame_t frame);
#define IRQD_GP             13
extern void interrupt_handler_protection();
void handler_protection(interrupt_frame_t frame);
#define IRQE_PF             14
extern void interrupt_handler_page();
void handler_page(interrupt_frame_t frame);
#define IRQF_RTN            15

#define IRQ10_MF            16
extern void interrupt_handler_fpu();
void handler_fpu(interrupt_frame_t frame);
#define IRQ11_AC            17
extern void interrupt_handler_align();
void handler_align(interrupt_frame_t frame);
#define IRQ12_MC            18
extern void interrupt_handler_machine();
void handler_machine(interrupt_frame_t frame);
#define IRQ13_XM            19
extern void interrupt_handler_simd();
void handler_simd(interrupt_frame_t frame);
#define IRQ14_VE            20
extern void interrupt_handler_virtual();
void handler_virtual(interrupt_frame_t frame);
#define IRQ15_CP            21
extern void interrupt_handler_control();
void handler_control(interrupt_frame_t frame);

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