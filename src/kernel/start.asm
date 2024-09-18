[bits 16]

extern kernel_init
global _start
_start:
    mov ah, 0x06
    mov bh, 0x07
    mov cx, 0
    mov dx, 0x184f
    int 0x10

    mov ax, 0x03
    int 0x10
    jmp kernel_init