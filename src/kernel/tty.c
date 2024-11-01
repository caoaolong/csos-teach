__asm__(".code16gcc");
#include <kernel.h>

void show_string(char *str)
{
    for (char *p = str; *p != 0; p++) {
        char c = (*p & 0xFF);
        __asm__ __volatile__(
            "mov $0x0E, %%ah\n"
            "mov %[c], %%al\n\t"
            "int $0x10\n"
            :: [c]"r"(c): "%ah", "%al");
    }  
}