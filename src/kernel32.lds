SECTIONS
{
	PROVIDE(b_kernel = .);

    . = 0x00010000;

	PROVIDE(e_kernel = .);

	PROVIDE(b_text = .);

	.text : {
		*(.text)
	} 

	.rodata : {
		*(.rodata)
	}

	PROVIDE(e_text = .);

	PROVIDE(b_data = .);

	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}

	PROVIDE(e_data = .);

	PROVIDE(memory_start = .);
}