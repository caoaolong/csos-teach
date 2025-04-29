#include <mio.h>
#include <kernel.h>
#include <csos/memory.h>

uint8_t minb(uint32_t addr)
{
    uint32_t pde = read_cr3();
    reset_pde();
    uint8_t v = *((volatile uint8_t *)addr);
    write_cr3(pde);
    return v;
}

uint16_t minw(uint32_t addr)
{
    uint32_t pde = read_cr3();
    reset_pde();
    uint16_t v = *((volatile uint16_t *)addr);
    write_cr3(pde);
    return v;
}

uint32_t minl(uint32_t addr)
{
    uint32_t pde = read_cr3();
    reset_pde();
    uint32_t v = *((volatile uint32_t *)addr);
    write_cr3(pde);
    return v;
}

void moutb(uint32_t addr, uint8_t value)
{
    uint32_t pde = read_cr3();
    reset_pde();
    *((volatile uint8_t *)addr) = value;
    write_cr3(pde);
}

void moutw(uint32_t addr, uint16_t value)
{
    uint32_t pde = read_cr3();
    reset_pde();
    *((volatile uint16_t *)addr) = value;
    write_cr3(pde);
}

void moutl(uint32_t addr, uint32_t value)
{
    uint32_t pde = read_cr3();
    reset_pde();
    *((volatile uint32_t *)addr) = value;
    write_cr3(pde);
}