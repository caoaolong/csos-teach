SECTIONS
{
	PROVIDE(b_kernel = .);

    . = 0x00010000;

	PROVIDE(b_text = .);

	.text : {
		*(EXCLUDE_FILE(*kernel32/init/init_*) .text)
	} 

	.rodata : {
		*(EXCLUDE_FILE(*kernel32/init/init_*) .rodata)
	}

	. = ALIGN(0x1000);

	PROVIDE(e_text = .);

	PROVIDE(b_data = .);

	.data : {
		*(EXCLUDE_FILE(*kernel32/init/init_*) .data)
	}
	.bss : {
		*(EXCLUDE_FILE(*kernel32/init/init_*) .bss)
	}

	e_data = .;

	. = 0x1FF6000;

	.init_task : AT(e_data) {
		*init_task_entry*(.text .rodata .data .bss)
		*init_task*(.text .rodata .data .bss)
	}

	PROVIDE(b_init_task = LOADADDR(.init_task));

	PROVIDE(e_init_task = LOADADDR(.init_task) + SIZEOF(.init_task));

	PROVIDE(memory_start = e_init_task);
}