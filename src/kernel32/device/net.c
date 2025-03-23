#include <net.h>
#include <pic.h>
#include <logf.h>
#include <interrupt.h>

void handler_net(interrupt_frame_t* frame)
{
    logf("%d", frame->code);
    send_eoi(IRQ1_NET);
}

void net_init()
{
    install_interrupt_handler(IRQ1_NET, (uint32_t)interrupt_handler_net);
    irq_enable(IRQ1_NET);
}