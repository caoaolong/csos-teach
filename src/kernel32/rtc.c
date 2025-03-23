#include <rtc.h>
#include <pic.h>
#include <logf.h>
#include <kernel.h>
#include <interrupt.h>
#include <csos/stdlib.h>
#include <csos/time.h>

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
    uint8_t vc = cmos_read(CMOS_C);
    create_alarm(1);

    if (vc & CMOS_C_AF)
        logf("A");
    else if (vc & CMOS_C_PF)
        logf(".");
}

void create_alarm(uint32_t value)
{
    tm time;
    time_read(&time, OS_TZ);

    uint8_t sec = value % 60;
    value /= 60;
    uint8_t min = value % 60;
    value /= 60;
    uint32_t hour = value;

    time.tm_sec += sec;
    if (time.tm_sec >= 60){
        time.tm_sec %= 60;
        time.tm_min += 1;
    }

    time.tm_min += min;
    if (time.tm_min >= 60){
        time.tm_min %= 60;
        time.tm_hour += 1;
    }

    time.tm_hour += (int)hour;
    if (time.tm_hour >= 24) {
        time.tm_hour %= 24;
    }

    cmos_write(CMOS_HOUR, bin_to_bcd(time.tm_hour - OS_TZ));
    cmos_write(CMOS_MINUTE, bin_to_bcd(time.tm_min));
    cmos_write(CMOS_SECOND, bin_to_bcd(time.tm_sec));
}

void rtc_init()
{
    // 周期、闹钟中断
    uint8_t prev = cmos_read(CMOS_B);
    cmos_write(CMOS_B, prev | CMOS_B_24HOUR | CMOS_B_AIE | CMOS_B_PIE);
    cmos_read(CMOS_C);
    // 设置闹钟
    create_alarm(1);
    // 设置中断频率
    prev = cmos_read(CMOS_A);
    cmos_write(CMOS_A, (prev &0xF0) | 0b1111);

    install_interrupt_handler(IRQ1_RTC, (uint32_t)interrupt_handler_rtc);
    irq_enable(IRQ1_RTC);
    irq_enable(IRQ0_CASCADE);
}