#include <kernel.h>
#include <pic.h>

void pic_init()
{
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC0_ICW2, IRQ_PIC_START);
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC0_ICW4, PIC_ICW4_8086);

    outb(PIC1_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    outb(PIC1_ICW3, 2);
    outb(PIC1_ICW4, PIC_ICW4_8086);

    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    outb(PIC1_IMR, 0xFF);
}

void irq_enable(int irq_number)
{
    if (irq_number < IRQ_PIC_START) return;

    irq_number -= IRQ_PIC_START;
    if (irq_number < 8) {
        uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_number);
        outb(PIC0_IMR, mask);
    } else {
        irq_number -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_number);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable(int irq_number)
{
    if (irq_number < IRQ_PIC_START) return;

    irq_number -= IRQ_PIC_START;
    if (irq_number < 8) {
        uint8_t mask = inb(PIC0_IMR) | (1 << irq_number);
        outb(PIC0_IMR, mask);
    } else {
        irq_number -= 8;
        uint8_t mask = inb(PIC1_IMR) | (1 << irq_number);
        outb(PIC1_IMR, mask);
    }
}

void send_eoi(int irq_number)
{
    int value = irq_number - IRQ_PIC_START;
    if (value >= 8) {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }
    outb(PIC0_OCW2, PIC_OCW2_EOI);
}