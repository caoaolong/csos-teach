[bits 16]

extern kernel_init
global _start
_start:
    mov ax, 0x3
    int 0x10
    jmp kernel_init