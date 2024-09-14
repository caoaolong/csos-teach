[org 0x7C00]

; edi存储内存位置
mov edi, 0x8000
; ecx存储起始扇区位置
mov ecx, 0
; bl存储扇区数量
mov bl, 1
call read_disk

xchg bx, bx

read_disk:
    pushad; ax, cx, dx, bx, sp, bp, si, di

    ; 设置扇区数量为1
    mov dx, 0x1F2
    mov al, bl
    out dx, al

    ; 设置起始扇区为0
    mov al, cl
    inc dx  ; 0x1F3
    out dx, al ; 起始扇区低八位

    shr ecx, 8
    mov al, cl
    inc dx  ; 0x1F4
    out dx, al ; 起始扇区中八位

    shr ecx, 8
    mov al, cl
    inc dx  ; 0x1F5
    out dx, al ; 起始扇区高八位

    shr ecx, 8
    and cl, 0xF
    inc dx  ; 0x1F6
    ; 设置为LBA模式
    mov al, 0xE0
    or al, cl
    out dx, al

    inc dx  ; 0x1F7
    ; 设置为读硬盘模式
    mov al, 0x20
    out dx, al

    xor ecx, ecx
    mov cl, bl

    .read:
        push cx
        call .waits
        call .reads
        pop cx
        loop .read
        popad
        ret

    .waits:
        mov dx, 0x1F7
        ; 检测硬盘状态
        .check:
            nop
            nop
            nop
            in al, dx
            ; 检测是否数据准备完毕
            and al, 0b1000_1000
            cmp al, 0b1000
            jnz .check
        ret

    .reads:
        mov dx, 0x1F0
        mov cx, 0x100     ; 512byte
        .readw:
            nop
            nop
            nop
            in ax, dx
            mov [edi], ax
            add edi, 2
            loop .readw
        ret

times 510 - ($ - $$) db 0
db 0x55, 0xAA