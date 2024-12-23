.code32
.text
.global default_task_entry
.extern default_entry
default_task_entry:
	mov %ss, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	jmp default_entry