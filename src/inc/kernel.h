#ifndef CSOS_KERNEL_H
#define CSOS_KERNEL_H

#include <x16/os.h>
#include <types.h>

#define KERNEL_CODE_SEG         (1 * 8)
#define KERNEL_DATA_SEG         (2 * 8)

/*==================Memory Check==================*/

#define MEMORY_MAX_COUNT    24

typedef struct memory_raw_t
{
    uint32_t base_l;
    uint32_t base_h;
    uint32_t length_l;
    uint32_t length_h;
    uint32_t type;
} memory_raw_t;

typedef struct memory_info_t
{
    memory_raw_t raws[MEMORY_MAX_COUNT];
    uint32_t count;
} memory_info_t;

/*================== GDT ==================*/

#define GDT_SIZE            0x100

#define SEG_ATTR_G          (1 << 15)
#define SEG_ATTR_D          (1 << 14)
#define SEG_ATTR_P          (1 << 7)

#define SEG_ATTR_DPL0       (0 << 5)
#define SEG_ATTR_DPL3       (3 << 5)

#define SEG_ATTR_CPL0       (0 << 0)
#define SEG_ATTR_CPL3       (3 << 0)

#define SEG_SYSTEM          (0 << 4)
#define SEG_NORMAL          (1 << 4)
#define SEG_TYPE_DATA       (0 << 3)
#define SEG_TYPE_CODE       (1 << 3)
#define SEG_TYPE_RW         (1 << 1)
#define SEG_TYPE_TSS        (9 << 0)

typedef struct gdt_table_t
{
    uint16_t limit_l;
    uint16_t base_l;
    uint8_t base_m;
    uint16_t attr;
    uint8_t base_h;
} _packed gdt_table_t;

/*==================Extern Functions==================*/

// 内存检测
void memory_check();
// 输出字符串
void show_string(char *str);
// 初始化GDT
void init_gdt();

/*==================Inline or static Functions==================*/

static inline uint8_t inb(uint16_t  port) {
	uint8_t rv;
	__asm__ volatile("inb %[p], %[v]" : [v]"=a" (rv) : [p]"d"(port));
	return rv;
}

static inline uint16_t inw(uint16_t  port) {
	uint16_t rv;
	__asm__ volatile("in %1, %0" : "=a" (rv) : "dN" (port));
	return rv;
}

static inline void outb(uint16_t port, uint8_t data) {
	__asm__ volatile("outb %[v], %[p]" : : [p]"d" (port), [v]"a" (data));
}

static inline void outw(uint16_t  port, uint16_t data) {
	__asm__ volatile("outb %[v], %[p]" : : [p]"d" (port), [v]"a" (data));
}

static inline void cli() {
	__asm__ volatile("cli");
}

static inline void sti() {
	__asm__ volatile("sti");
}

static inline void lgdt(uint32_t start, uint32_t size) {
    struct {
		uint16_t limit;
		uint16_t start_l;
		uint16_t start_h;
	} gdt;

	gdt.start_h = start >> 16;
	gdt.start_l = start & 0xFFFF;
	gdt.limit = size - 1;
	__asm__ volatile("lgdt %[g]"::[g]"m"(gdt));
}

static inline uint32_t read_cr0() {
	uint32_t cr0;
	__asm__ volatile("mov %%cr0, %[v]":[v]"=r"(cr0));
	return cr0;
}

static inline void write_cr0(uint32_t v) {
	__asm__ volatile("mov %[v], %%cr0"::[v]"r"(v));
}

static inline void far_jump(uint32_t selector, uint32_t offset) {
	uint32_t addr[] = { offset, selector };
	__asm__ volatile("ljmpl *(%[a])"::[a]"r"(addr));
}

void protect_mode();

#endif