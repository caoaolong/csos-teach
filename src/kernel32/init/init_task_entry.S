.code32
.text
.global init_task_entry
.extern init_entry
init_task_entry:
	mov %ss, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	jmp init_entry