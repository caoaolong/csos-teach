SECTIONS {
    . = 0x4F000000;
    .text : {
        *(*.text)
    }
    .rodata : {
        *(*.rodata)
    }
    .data : {
        *(*.data)
    }
    .bss : {
        *(*.bss)
    }
}