#include <interrupt.h>
#include <kernel.h>
#include <os.h>
#include <logf.h>
#include <pic.h>
#include <timer.h>
#include <rtc.h>
#include <csos/syscall.h>
#include <kbd.h>

gdt_gate_t int_table[INTERRUPT_GATE_SIZE];

void set_interrupt_gate(int vector, uint32_t offset, uint32_t selector, uint16_t attr)
{
    gdt_gate_t *entry = &int_table[vector];
    entry->offset_l = offset & 0xFFFF;
    entry->selector = selector;
    entry->attr = attr;
    entry->offset_h = (offset >> 16) & 0xFFFF;
}

void handler_default(interrupt_frame_t *frame)
{
    logf("default handler");
    while (TRUE) HLT;
}

void handler_division(interrupt_frame_t *frame)
{
    logf("division handler");
    while (TRUE) HLT;
}

void handler_debug(interrupt_frame_t *frame)
{
    logf("debug handler");
    while (TRUE) HLT;
}

void handler_nmi(interrupt_frame_t *frame)
{
    logf("nmi handler");
    while (TRUE) HLT;
}

void handler_breakpoint(interrupt_frame_t *frame)
{
    logf("breakpoint handler");
    while (TRUE) HLT;
}

void handler_overflow(interrupt_frame_t *frame)
{
    logf("overflow handler");
    while (TRUE) HLT;
}

void handler_range(interrupt_frame_t *frame)
{
    logf("range handler");
    while (TRUE) HLT;
}

void handler_opcode(interrupt_frame_t *frame)
{
    logf("opcode handler");
    while (TRUE) HLT;
}

void handler_device(interrupt_frame_t *frame)
{
    logf("device handler");
    while (TRUE) HLT;
}

void handler_double(interrupt_frame_t *frame)
{
    logf("double handler");
    while (TRUE) HLT;
}

void handler_tss(interrupt_frame_t *frame)
{
    logf("tss handler");
    while (TRUE) HLT;
}

void handler_segment(interrupt_frame_t *frame)
{
    logf("segment handler");
    while (TRUE) HLT;
}

void handler_stack(interrupt_frame_t *frame)
{
    logf("stack handler");
    while (TRUE) HLT;
}

void handler_protection(interrupt_frame_t *frame)
{
    uint32_t la = read_cr2();
    logf("#GP:");
    if (!frame->code & ERROR_PROT_EXT) {
        logf("  ERR     : ext");
    }

    if (!frame->code & ERROR_PROT_IDT) {
        logf("  ERR     : idt");
    }
    logf("  SELECTOR: %08x", frame->code & 0xFFF8);
    while (TRUE) HLT;
}

void handler_page(interrupt_frame_t *frame)
{
    uint32_t la = read_cr2();
    logf("#PF:");
    if (frame->code & ERROR_PAGE_P) {
        logf("  ERR: page-level protection: %08X", la);
    } else {
        logf("  ERR: Non-Present          : %08X", la);
    }

    if (frame->code & ERROR_PAGE_WR) {
        logf("  ERR: write                : %08X", la);
    } else {
        logf("  ERR: read                 : %08X", la);
    }

    if (frame->code & ERROR_PAGE_US) {
        logf("  ERR: user-mode            : %08X", la);
    } else {
        logf("  ERR: supervisor-mode      : %08X", la);
    }
    while (TRUE) HLT;
}

void handler_fpu(interrupt_frame_t *frame)
{
    logf("fpu handler");
    while (TRUE) HLT;
}

void handler_align(interrupt_frame_t *frame)
{
    logf("align handler");
    while (TRUE) HLT;
}

void handler_machine(interrupt_frame_t *frame)
{
    logf("machine handler\n");
    while (TRUE) HLT;
}

void handler_simd(interrupt_frame_t *frame)
{
    logf("simd handler");
    while (TRUE) HLT;
}

void handler_virtual(interrupt_frame_t *frame)
{
    logf("virtual handler");
    while (TRUE) HLT;
}

void handler_control(interrupt_frame_t *frame)
{
    logf("control handler");
    while (TRUE) HLT;
}

void install_interrupt_handler(int vector, uint32_t handler)
{
    install_interrupt_handler_dpl(vector, handler, GATE_ATTR_DPL0);
}

void install_interrupt_handler_dpl(int vector, uint32_t handler, uint16_t dpl)
{
    set_interrupt_gate(vector, handler, KERNEL_CODE_SEG, 
            GATE_ATTR_P | dpl | GATE_TYPE_IDT);
}

void interrupt_init()
{
    for (int i = 0; i < INTERRUPT_GATE_SIZE; i++)
    {
        set_interrupt_gate(i, (uint32_t)interrupt_handler_default, 
            KERNEL_CODE_SEG, 
            GATE_ATTR_P | GATE_ATTR_DPL0 | GATE_TYPE_IDT);
    }
    install_interrupt_handler(IRQ0_DE, (uint32_t)interrupt_handler_division);
    install_interrupt_handler(IRQ1_DB, (uint32_t)interrupt_handler_debug);
    install_interrupt_handler(IRQ2_NMI, (uint32_t)interrupt_handler_nmi);
    install_interrupt_handler(IRQ3_BP, (uint32_t)interrupt_handler_breakpoint);
    install_interrupt_handler(IRQ4_OF, (uint32_t)interrupt_handler_overflow);
    install_interrupt_handler(IRQ5_BR, (uint32_t)interrupt_handler_range);
    install_interrupt_handler(IRQ6_UD, (uint32_t)interrupt_handler_opcode);
    install_interrupt_handler(IRQ7_NM, (uint32_t)interrupt_handler_device);
    install_interrupt_handler(IRQ8_DF, (uint32_t)interrupt_handler_double);
    install_interrupt_handler(IRQA_TS, (uint32_t)interrupt_handler_tss);
    install_interrupt_handler(IRQB_NP, (uint32_t)interrupt_handler_segment);
    install_interrupt_handler(IRQC_SS, (uint32_t)interrupt_handler_stack);
    install_interrupt_handler(IRQD_GP, (uint32_t)interrupt_handler_protection);
    install_interrupt_handler(IRQE_PF, (uint32_t)interrupt_handler_page);
    install_interrupt_handler(IRQ10_MF, (uint32_t)interrupt_handler_fpu);
    install_interrupt_handler(IRQ11_AC, (uint32_t)interrupt_handler_align);
    install_interrupt_handler(IRQ12_MC, (uint32_t)interrupt_handler_machine);
    install_interrupt_handler(IRQ13_XM, (uint32_t)interrupt_handler_simd);
    install_interrupt_handler(IRQ14_VE, (uint32_t)interrupt_handler_virtual);
    install_interrupt_handler(IRQ15_CP, (uint32_t)interrupt_handler_control);

    install_interrupt_handler_dpl(IRQ_SYSCALL, (uint32_t)interrupt_handler_syscall, GATE_ATTR_DPL3);

    lidt((uint32_t)int_table, sizeof(int_table));

    pic_init();
    timer_init();
    // rtc_init();
    kbd_init();
}

protect_state_t protect_enter()
{
    uint32_t eflags = read_eflags();
    cli();
    return eflags;
}

protect_state_t protect_exit(protect_state_t ps)
{
    write_eflags(ps);
}