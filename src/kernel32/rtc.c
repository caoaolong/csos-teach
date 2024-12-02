#include <rtc.h>
#include <pic.h>
#include <tty.h>
#include <kernel.h>
#include <interrupt.h>

uint8_t cmos_read(uint8_t addr)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    return inb(CMOS_DATA);
}

void cmos_write(uint8_t addr, uint8_t value)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    outb(CMOS_DATA, value);
}

static uint32_t volatile counter = 0;

void handler_rtc(interrupt_frame_t* frame)
{
    send_eoi(IRQ1_RTC);
    cmos_read(CMOS_C);
    tty_printf("rtc handler %d ...\n", counter++);
}

void set_alarm(uint32_t value)
{
    
}

void rtc_init()
{
    // 周期中断、闹钟中断
    cmos_write(CMOS_B, 0b01100010);
    cmos_read(CMOS_C);
    // 设置中断频率
    outb(CMOS_A, (inb(CMOS_A) & 0xF) | 0b1110);

    install_interrupt_handler(IRQ1_RTC, (uint32_t)interrupt_handler_rtc);
    irq_enable(IRQ1_RTC);
    irq_enable(IRQ0_CASCADE);
}