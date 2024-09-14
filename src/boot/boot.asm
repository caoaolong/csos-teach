[bits 16]

_start:
    mov ax, 0
    mov es, ax
; 读取内核代码
read_disk:
	; ax=目标地址
	mov bx, 0x7e00
	; ch=磁道号 cl=扇区号
	mov cx, 0x02
	; ah=2表示读扇区,3表示写扇区 al=读取/写入的扇区数
	mov ax, 0x201
	; dh=磁头号 dl=驱动器号(软驱从0开始, 硬盘从80H开始)
	mov dx, 0x80
	int 0x13
	jc read_disk
    xchg bx, bx
    xchg bx, bx
	jmp 0:0x7e00

times 510 - ($ - $$) db 0
dw 0xaa55