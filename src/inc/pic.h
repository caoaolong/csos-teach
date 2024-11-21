#ifndef CSOS_PIC_H
#define CSOS_PIC_H

#define PIC0_ICW1   0x20
#define PIC0_ICW2   0x21
#define PIC0_ICW3   0x21
#define PIC0_ICW4   0x21
#define PIC0_OCW2	0x20
#define PIC0_IMR    0x21

#define PIC1_ICW1   0xA0
#define PIC1_ICW2   0xA1
#define PIC1_ICW3   0xA1
#define PIC1_ICW4   0xA1
#define PIC1_OCW2	0xA0
#define PIC1_IMR    0xA1

#define PIC_ICW1_ALWAYS_1   (1 << 4)
#define PIC_ICW1_ICW4       (1 << 0)
#define PIC_ICW4_8086       (1 << 0)
#define PIC_OCW2_EOI		(1 << 5)
#define IRQ_PIC_START       0x20

void pic_init();

void irq_enable(int irq_number);

void irq_disable(int irq_number);

void send_eoi(int irq_number);

#endif