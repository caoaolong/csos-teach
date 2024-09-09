[org 0x7c00]

start:
    mov ax, 0x03
    int 0x10

    mov si, msg
    call print
    jmp $

print:
    mov ah, 0x0e
    .nc:
        lodsb
        cmp al, 0
        je .done
        int 0x10
        jmp .nc
    .done:
        ret

msg db 'Hello,World!', 0

times 510 - ($ - $$) db 0
dw 0xaa55