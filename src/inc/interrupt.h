#ifndef CSOS_INTERRUPT_H
#define CSOS_INTERRUPT_H

#include <types.h>

typedef struct interrupt_frame_t
{
    // 手动压入
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t code;
    // 自动压入
    uint32_t eip, cs, eflags;
    uint32_t esp3, ss3; 
} interrupt_frame_t;

// 中断向量及处理函数
extern void interrupt_handler_default();
void handler_default(interrupt_frame_t *frame);
#define IRQ0_DE             0
extern void interrupt_handler_division();
void handler_division(interrupt_frame_t *frame);
#define IRQ1_DB             1
extern void interrupt_handler_debug();
void handler_debug(interrupt_frame_t *frame);
#define IRQ2_NMI            2
extern void interrupt_handler_nmi();
void handler_nmi(interrupt_frame_t *frame);
#define IRQ3_BP             3
extern void interrupt_handler_breakpoint();
void handler_breakpoint(interrupt_frame_t *frame);
#define IRQ4_OF             4
extern void interrupt_handler_overflow();
void handler_overflow(interrupt_frame_t *frame);
#define IRQ5_BR             5
extern void interrupt_handler_range();
void handler_range(interrupt_frame_t *frame);
#define IRQ6_UD             6
extern void interrupt_handler_opcode();
void handler_opcode(interrupt_frame_t *frame);
#define IRQ7_NM             7
extern void interrupt_handler_device();
void handler_device(interrupt_frame_t *frame);
#define IRQ8_DF             8
extern void interrupt_handler_double();
void handler_double(interrupt_frame_t *frame);
#define IRQ9_CSO            9

#define IRQA_TS             10
extern void interrupt_handler_tss();
void handler_tss(interrupt_frame_t *frame);
#define IRQB_NP             11
extern void interrupt_handler_segment();
void handler_segment(interrupt_frame_t *frame);
#define IRQC_SS             12
extern void interrupt_handler_stack();
void handler_stack(interrupt_frame_t *frame);
#define IRQD_GP             13
extern void interrupt_handler_protection();
void handler_protection(interrupt_frame_t *frame);
#define IRQE_PF             14
extern void interrupt_handler_page();
void handler_page(interrupt_frame_t *frame);
#define IRQF_RTN            15

#define IRQ10_MF            16
extern void interrupt_handler_fpu();
void handler_fpu(interrupt_frame_t *frame);
#define IRQ11_AC            17
extern void interrupt_handler_align();
void handler_align(interrupt_frame_t *frame);
#define IRQ12_MC            18
extern void interrupt_handler_machine();
void handler_machine(interrupt_frame_t *frame);
#define IRQ13_XM            19
extern void interrupt_handler_simd();
void handler_simd(interrupt_frame_t *frame);
#define IRQ14_VE            20
extern void interrupt_handler_virtual();
void handler_virtual(interrupt_frame_t *frame);
#define IRQ15_CP            21
extern void interrupt_handler_control();
void handler_control(interrupt_frame_t *frame);

#define IRQ0_TIMER          0x20
extern void interrupt_handler_timer();
void handler_timer(interrupt_frame_t* frame);
#define IRQ1_KBD            0x21
extern void interrupt_handler_kbd();
void handler_kbd(interrupt_frame_t* frame);
#define IRQ1_RTC            0x28
extern void interrupt_handler_rtc();
void handler_rtc(interrupt_frame_t* frame);

#define IRQ_SYSCALL         0x80
extern void interrupt_handler_syscall();
void handler_syscall(interrupt_frame_t* frame);

#define IRQ0_CASCADE        0x22

// 缺页异常标志
#define ERROR_PAGE_P    (1 << 0)
#define ERROR_PAGE_WR   (1 << 1)
#define ERROR_PAGE_US   (1 << 2)

// 正常保护异常标志
#define ERROR_PROT_EXT  (1 << 0)
#define ERROR_PROT_IDT  (1 << 1)

#define INTERRUPT_GATE_SIZE 0x100

void install_interrupt_handler(int vector, uint32_t handler);

void install_interrupt_handler_dpl(int vector, uint32_t handler, uint16_t dpl);

typedef uint32_t protect_state_t;

protect_state_t protect_enter();

protect_state_t protect_exit(protect_state_t ps);

void interrupt_init();

#endif