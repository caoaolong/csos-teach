#include "os.h"

.extern kernel_init, kernel32_init

.code16
.text
.global _start
_start: 
	mov $0x3, %ax
	int $0x10
	jmp kernel_init

.code32
.text
.global protect_mode
protect_mode:
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	jmp $KERNEL_CODE_SEG, $kernel32_init