#include <csos/stdlib.h>

#define SECTOR_SIZE 512

uint8_t bcd_to_bin(uint8_t value)
{
    return (value & 0xF) + (value >> 4) * 10;
}

uint8_t bin_to_bcd(uint8_t value)
{
    return ((value / 10) << 4) + (value % 10);
}

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

int strings_count(char *argv[])
{
    int count = 0;
    if (argv)
        while (*argv ++) count ++;
    return count;
}

char *get_file_name(char *name)
{
    char *s = name;
    while (*s != '\0') s++;
    while ((*s != '/') && (*s != '\\') && (s >= name)) s--;
    return s + 1;
}