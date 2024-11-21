#include <timer.h>
#include <pic.h>
#include <kernel.h>

static uint32_t ticks = 0;

void handler_timer(interrupt_frame_t* frame)
{
    ticks ++;
    send_eoi(IRQ0_TIMER);
}

void timer_init()
{
    uint32_t reload_count = PIT_OSC_FREQ * OS_TICKS_MS / 1000;
    outb(PIT_CMD_MODE_PORT, PIT_CHANNLE0 | PIT_LOAD_LOHI | PIT_MODE0);
    outb(PIT_CHL0_DATA_PORT, reload_count & 0xFF); 
    outb(PIT_CHL0_DATA_PORT, (reload_count >> 8) & 0xFF);

    install_interrupt_handler(IRQ0_TIMER, (uint32_t)interrupt_handler_timer);
    irq_enable(IRQ0_TIMER);
}