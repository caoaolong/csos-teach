#include <net.h>
#include <pic.h>
#include <logf.h>
#include <interrupt.h>

static e1000_t e1000;

void handler_net(interrupt_frame_t* frame)
{
    logf("%d", frame->code);
    send_eoi(IRQ1_NET);
}

static void e1000_read_mac()
{
    
}

static void e1000_reset()
{

}

void net_init()
{
    install_interrupt_handler(IRQ1_NET, (uint32_t)interrupt_handler_net);
    irq_enable(IRQ1_NET);
}