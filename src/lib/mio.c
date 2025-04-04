#include <mio.h>

uint8_t minb(uint32_t addr)
{
    return *((volatile uint8_t *)addr);
}

uint16_t minw(uint32_t addr)
{
    return *((volatile uint16_t *)addr);
}

uint32_t minl(uint32_t addr)
{
    return *((volatile uint32_t *)addr);
}

void moutb(uint32_t addr, uint8_t value)
{
    *((volatile uint8_t *)addr) = value;
}

void moutw(uint32_t addr, uint16_t value)
{
    *((volatile uint16_t *)addr) = value;
}

void moutl(uint32_t addr, uint32_t value)
{
    *((volatile uint32_t *)addr) = value;
}