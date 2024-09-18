__asm__(".code16gcc");
#include <x16/kernel.h>

memory_info_t memory_info;

void memory_check()
{
    int sign32 = 0x534D4150, sign16 = 0xE820;
    int index = 0, signature, bytes;
    while (TRUE) {
        memory_raw_t *memory_raw = &memory_info.raws[index];
        __asm__ __volatile__("int $0x15" 
				: "=a"(signature), "=c"(bytes), "=b"(index)
				: "a"(sign16), "b"(index), "c"(MEMORY_MAX_COUNT), "d"(sign32), "D"(memory_raw));
        if (signature != sign32) {
            show_string("Memory check error!\r\n");
        }
        memory_info.count ++;
        if (index == 0) {
            show_string("Memory check success!\r\n");
            break;
        }
    }
}