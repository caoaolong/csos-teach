#include <interrupt.h>
#include <kernel.h>
#include <os.h>
#include <tty.h>

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

void installinterrupt_handler(int vector, uint32_t handler)
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
    installinterrupt_handler(IRQ0_DE, (uint32_t)interrupt_handler_division);
    installinterrupt_handler(IRQ1_DB, (uint32_t)interrupt_handler_debug);
    installinterrupt_handler(IRQ2_NMI, (uint32_t)interrupt_handler_nmi);
    installinterrupt_handler(IRQ3_BP, (uint32_t)interrupt_handler_breakpoint);
    installinterrupt_handler(IRQ4_OF, (uint32_t)interrupt_handler_overflow);
    installinterrupt_handler(IRQ5_BR, (uint32_t)interrupt_handler_range);
    installinterrupt_handler(IRQ6_UD, (uint32_t)interrupt_handler_opcode);
    installinterrupt_handler(IRQ7_NM, (uint32_t)interrupt_handler_device);
    installinterrupt_handler(IRQ8_DF, (uint32_t)interrupt_handler_double);
    installinterrupt_handler(IRQA_TS, (uint32_t)interrupt_handler_tss);
    installinterrupt_handler(IRQB_NP, (uint32_t)interrupt_handler_segment);
    installinterrupt_handler(IRQC_SS, (uint32_t)interrupt_handler_stack);
    installinterrupt_handler(IRQD_GP, (uint32_t)interrupt_handler_protection);
    installinterrupt_handler(IRQE_PF, (uint32_t)interrupt_handler_page);
    installinterrupt_handler(IRQ10_MF, (uint32_t)interrupt_handler_fpu);
    installinterrupt_handler(IRQ11_AC, (uint32_t)interrupt_handler_align);
    installinterrupt_handler(IRQ12_MC, (uint32_t)interrupt_handler_machine);
    installinterrupt_handler(IRQ13_XM, (uint32_t)interrupt_handler_simd);
    installinterrupt_handler(IRQ14_VE, (uint32_t)interrupt_handler_virtual);
    installinterrupt_handler(IRQ15_CP, (uint32_t)interrupt_handler_control);

    lidt((uint32_t)int_table, INTERRUPT_GATE_SIZE);
}