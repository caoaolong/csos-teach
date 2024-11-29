#include <interrupt.h>
#include <kernel.h>
#include <os.h>
#include <tty.h>
#include <pic.h>
#include <timer.h>

gate_t int_table[INTERRUPT_GATE_SIZE];

void set_interrupt_gate(int vector, uint32_t offset, uint32_t selector, uint16_t attr)
{
    gate_t *entry = &int_table[vector];
    entry->offset_l = offset & 0xFFFF;
    entry->selector = selector;
    entry->attr = attr;
    entry->offset_h = (offset >> 16) & 0xFFFF;
}

void handler_default(interrupt_frame_t frame)
{
    tty_printf("default handler\n");
    while (TRUE)
    {
        HLT;
    }
}

void handler_division(interrupt_frame_t frame)
{
    tty_printf("division handler\n");
}

void handler_debug(interrupt_frame_t frame)
{
    tty_printf("debug handler\n");
}

void handler_nmi(interrupt_frame_t frame)
{
    tty_printf("nmi handler\n");
}

void handler_breakpoint(interrupt_frame_t frame)
{
    tty_printf("breakpoint handler\n");
}

void handler_overflow(interrupt_frame_t frame)
{
    tty_printf("overflow handler\n");
}

void handler_range(interrupt_frame_t frame)
{
    tty_printf("range handler\n");
}

void handler_opcode(interrupt_frame_t frame)
{
    tty_printf("opcode handler\n");
}

void handler_device(interrupt_frame_t frame)
{
    tty_printf("device handler\n");
}

void handler_double(interrupt_frame_t frame)
{
    tty_printf("double handler\n");
}

void handler_tss(interrupt_frame_t frame)
{
    tty_printf("tss handler\n");
}

void handler_segment(interrupt_frame_t frame)
{
    tty_printf("segment handler\n");
}

void handler_stack(interrupt_frame_t frame)
{
    tty_printf("stack handler\n");
}

void handler_protection(interrupt_frame_t frame)
{
    tty_printf("protection handler\n");
}

void handler_page(interrupt_frame_t frame)
{
    tty_printf("page handler\n");
}

void handler_fpu(interrupt_frame_t frame)
{
    tty_printf("fpu handler\n");
}

void handler_align(interrupt_frame_t frame)
{
    tty_printf("align handler\n");
}

void handler_machine(interrupt_frame_t frame)
{
    tty_printf("machine handler\n");
}

void handler_simd(interrupt_frame_t frame)
{
    tty_printf("simd handler\n");
}

void handler_virtual(interrupt_frame_t frame)
{
    tty_printf("virtual handler\n");
}

void handler_control(interrupt_frame_t frame)
{
    tty_printf("control handler\n");
}

void install_interrupt_handler(int vector, uint32_t handler)
{
    set_interrupt_gate(vector, handler, KERNEL_CODE_SEG, 
            GATE_ATTR_P | GATE_ATTR_DPL0 | GAT_TYPE_INT);
}

void interrupt_init()
{
    for (int i = 0; i < INTERRUPT_GATE_SIZE; i++)
    {
        set_interrupt_gate(i, (uint32_t)interrupt_handler_default, 
            KERNEL_CODE_SEG, 
            GATE_ATTR_P | GATE_ATTR_DPL0 | GAT_TYPE_INT);
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

    lidt((uint32_t)int_table, sizeof(int_table));

    pic_init();
    timer_init();
}