.text
.global _start
.extern csos_init
_start:
    push %ebp
    mov %esp, %ebp

    mov 0xC(%ebp), %eax
    push %eax
    mov 0x8(%ebp), %eax
    push %eax
    call csos_init
    
    jmp .

.text
.global simple_switch
simple_switch:
    mov 4(%esp), %ecx
    mov 8(%esp), %edx
    push %ebp
    push %ebx
    push %esi
    push %edi
    pushf
    mov %esp, (%ecx)
    mov %edx, %esp
    popf
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    ret

.global syscall_handler
syscall_handler:
    ret

.macro interrupt_handler func vector code
.text
.global interrupt_handler_\func
.extern handler_\func
interrupt_handler_\func:
    .if \code == 0
        push $0
    .endif
    push $\vector

    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    push %esp
    call handler_\func
    add $(1 * 4), %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa

    add $(2 * 4), %esp

    iret

.endm

interrupt_handler default,          -1,     0
interrupt_handler division,         0,      0
interrupt_handler debug,            1,      0
interrupt_handler nmi,              2,      0
interrupt_handler breakpoint,       3,      0
interrupt_handler overflow,         4,      0
interrupt_handler range,            5,      0
interrupt_handler opcode,           6,      0
interrupt_handler device,           7,      0
interrupt_handler double,           8,      1
interrupt_handler tss,              10,     1
interrupt_handler segment,          11,     1
interrupt_handler stack,            12,     1
interrupt_handler protection,       13,     1
interrupt_handler page,             14,     1
interrupt_handler fpu,              16,     0
interrupt_handler align,            17,     1
interrupt_handler machine,          18,     0
interrupt_handler simd,             19,     0
interrupt_handler virtual,          20,     0
interrupt_handler control,          21,     1

interrupt_handler timer,            0x20,   0
interrupt_handler rtc,              0x28,   0