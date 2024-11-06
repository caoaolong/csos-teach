#include <kernel.h>

#define SECTOR_SIZE 512
#define OS_ADDR     0x100000

void read_disk(uint32_t sector, uint32_t count, uint16_t* buffer)
{
    outb(0x1F6, 0xE0);
    outb(0x1F2, (uint8_t)(count >> 8));
    outb(0x1F3, (uint8_t)(count >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    outb(0x1F2, (uint8_t)count);
    outb(0x1F3, (uint8_t)sector);
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));

    outb(0x1F7, 0x24);

    uint16_t *ptr = buffer;
    while(count --) {
        while((inb(0x1F7) & 0x88) != 0x8) ;
        for (int i = 0; i < SECTOR_SIZE / 2; i++) {
            *ptr++ = inw(0x1F0);
        }
    }
}

uint32_t read_elf_header(uint8_t *buffer)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)buffer;
    if (elf_header->e_ident[0] != 0x7F || 
        elf_header->e_ident[1] != 'E' || 
        elf_header->e_ident[2] != 'L' || 
        elf_header->e_ident[3] != 'F') {
        return 0;
    }

    for (int i = 0; i < elf_header->e_phnum; i++) {
        Elf32_Phdr *phdr = (Elf32_Phdr *)(buffer + elf_header->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        uint8_t *src = buffer + phdr->p_offset;
        uint8_t *dst = (uint8_t*) phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j++) {
            *dst++ = *src++;
        }
        dst = (uint8_t*)phdr->p_paddr + phdr->p_filesz;
        for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
            *dst++ = 0;
        }
    }

    return elf_header->e_entry;
}

void test(uint32_t a, uint32_t b)
{
    uint32_t c = a + b;
}

void kernel32_init()
{
    test(5, 6);
    // 0号扇区: 引导扇区
    // 1-9: Kernel x16
    // 10-*: Kernel x86
    // read_disk(10, 500, (uint16_t*)OS_ADDR);
    // uint32_t addr = read_elf_header((uint8_t*)OS_ADDR);
    // while (TRUE);
}