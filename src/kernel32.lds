SECTIONS
{
	PROVIDE(b_kernel = .);

    . = 0x00010000;

	PROVIDE(b_text = .);

	.text : {
		*(.text)
	} 

	.rodata : {
		*(.rodata)
	}

	. = ALIGN(0x1000);

	PROVIDE(e_text = .);

	PROVIDE(b_data = .);

	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}

	PROVIDE(memory_start = .);
}