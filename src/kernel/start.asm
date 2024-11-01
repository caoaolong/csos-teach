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


[bits 32]

KERNEL_CODE_SEG equ (1 * 8)
KERNEL_DATA_SEG equ (2 * 8)

extern kernel32_init
global protect_mode:
protect_mode:
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp KERNEL_CODE_SEG:kernel32_init