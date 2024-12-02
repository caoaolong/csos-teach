#include <timer.h>
#include <pic.h>
#include <kernel.h>
#include <tty.h>

static uint32_t ticks = 0;
static uint32_t beeping = 0;
static uint32_t frequency = 200;

void handler_timer(interrupt_frame_t* frame)
{
    if (beeping == 0)
        start_beep();
    if (beeping > frequency)
        stop_beep();
    beeping = beeping == frequency * 2 ? 0 : beeping + 5;

    ticks ++;
    send_eoi(IRQ0_TIMER);
    tty_printf("beeping: %d\n", beeping);
}

void start_beep()
{
    outb(SPEAKER_REG, inb(SPEAKER_REG) | 0b11);
}

void stop_beep()
{
    outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xFC);
}

void timer_init()
{
    // 配置计数器
    uint32_t reload_count = PIT_OSC_FREQ * OS_TICKS_MS / 1000;
    outb(PIT_CMD_MODE_PORT, PIT_CHANNLE0 | PIT_LOAD_LOHI | PIT_MODE0);
    outb(PIT_CHL0_DATA_PORT, reload_count & 0xFF); 
    outb(PIT_CHL0_DATA_PORT, (reload_count >> 8) & 0xFF);

    // 配置蜂鸣器
    outb(PIT_CMD_MODE_PORT, PIT_CHANNLE2 | PIT_LOAD_LOHI | PIT_MODE0);
    outb(PIT_CHL2_DATA_PORT, (uint8_t)BEEP_COUNTER);
    outb(PIT_CHL2_DATA_PORT, (uint8_t)BEEP_COUNTER >> 8);

    install_interrupt_handler(IRQ0_TIMER, (uint32_t)interrupt_handler_timer);
    irq_enable(IRQ0_TIMER);
}