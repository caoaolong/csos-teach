__asm__(".code16gcc");
#include <x16/kernel.h>

void show_string(char *str, uint8_t color)
{
    color = color & 0xFF;
    for (char *p = str; *p != 0; p++) {
        char c = (*p & 0xFF) | 0x0E00;
        __asm__ volatile("mov $0xE, %%ah\n"
                    "mov %[c], %%al\n\t"
                    "int $0x10\n"
                :: [c]"r"(c), "b"(color));
    }   
}