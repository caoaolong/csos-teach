#include <csos/stdlib.h>

uint8_t bcd_to_bin(uint8_t value)
{
    return (value & 0xF) + (value >> 4) * 10;
}

uint8_t bin_to_bcd(uint8_t value)
{
    return ((value / 10) << 4) + (value % 10);
}