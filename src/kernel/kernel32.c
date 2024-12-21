#include <kernel.h>
#include <paging.h>

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

#define CR4_PSE (1 << 4)
#define CR0_PG  (1 << 31)
#define PDE_P   (1 << 0)
#define PDE_W   (1 << 1)
#define PDE_PS  (1 << 7)

extern memory_info_t memory_info;
extern gdt_table_t gdt_table;

void enable_paging()
{
    static uint32_t page_dir[1024] __attribute__((aligned(PAGE_SIZE))) = {
        [0] = PDE_P | PDE_W | PDE_PS | 0
    };
    write_cr4(read_cr4() | CR4_PSE);
    write_cr3((uint32_t)page_dir);
    write_cr0(read_cr0() | CR0_PG);
}

void kernel32_init()
{
    // 0号扇区: 引导扇区
    // 1-64: Kernel x16
    // 65-*: Kernel x86
    read_disk(65, 500, (uint16_t*)OS_ADDR);
    uint32_t addr = read_elf_header((uint8_t*)OS_ADDR);
    if (addr == 0)
        while (TRUE);
    // 开启分页
    enable_paging();
    ((void (*)(memory_info_t*, uint32_t))addr)(&memory_info, (uint32_t)&gdt_table);
}