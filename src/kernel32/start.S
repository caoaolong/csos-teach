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
.global interrupt_handler_default
interrupt_handler_default:
    iret