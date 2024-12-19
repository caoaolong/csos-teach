#include <csos/memory.h>
#include <tty.h>
#include <paging.h>

#define CR4_PSE (1 << 4)
#define CR0_PG  (1 << 31)
#define PDE_P   (1 << 0)
#define PDE_W   (1 << 1)
#define PDE_PS  (1 << 7)

static void enable_paging()
{
    static uint32_t page_dir[1024] __attribute__((aligned(PAGE_SIZE))) = {
        [0] = PDE_P | PDE_W | PDE_PS | 0
    };
    write_cr4(read_cr4() | CR4_PSE);
    write_cr3((uint32_t)page_dir);
    write_cr0(read_cr0() | CR0_PG);
}

static void init_memory32_info(memory32_info_t *memory32_info, uint8_t *bits, 
    uint32_t start, uint32_t size, uint32_t page_size)
{
    mutex_init(&memory32_info->mutex);
    memory32_info->size = size;
    memory32_info->page_size = page_size;
    memory32_info->start = start;
    bitmap_init(&memory32_info->bitmap, bits, size / page_size, FLASE);
}

static uint32_t memory32_alloc_page(memory32_info_t *memory32_info, uint32_t page_size)
{
    mutex_lock(&memory32_info->mutex);
    uint32_t addr = 0;
    int page_index = bitmap_alloc_bits(&memory32_info->bitmap, FLASE, page_size);
    if (page_index >= 0)
    {
        addr = memory32_info->start + page_index * memory32_info->page_size;
    }
    mutex_unlock(&memory32_info->mutex);
    return addr;
}

static void memory32_free_page(memory32_info_t *memory32_info, uint32_t addr, uint32_t page_size)
{
    mutex_lock(&memory32_info->mutex);
    uint32_t page_index = (addr  - memory32_info->start) / memory32_info->page_size;
    bitmap_set_bits(&memory32_info->bitmap, page_index, page_size, FLASE);
    mutex_unlock(&memory32_info->mutex);
}

static void kernel_pagging()
{
    extern uint32_t b_text[], e_text[], b_data[], e_data[], b_kernel[], e_kernel[];

    static memory32_map_t kernel_map[] = {
        { b_kernel, e_kernel, b_kernel, 0 },
        { b_text, e_text, b_text, 0 },
        { b_data, e_data, b_data, 0 },
    };
    uint32_t count = sizeof(kernel_map) / sizeof(memory32_map_t);
    for (int i = 0; i < count; i++)
    {
        memory32_map_t *map = &kernel_map[i];
        uint32_t vstart = (uint32_t)map->vstart & 0xFFFFF000;
        uint32_t vlimit = (uint32_t)map->vlimit & 0xFFFFF000;
        uint32_t page_count = (vlimit - vstart) / PAGE_SIZE;
    }
}

#define MEMORY_EXT_START    0x100000

void memory_init(memory_info_t *memory_info)
{
    uint32_t total_size = 0;
    for (int i = 0; i < memory_info->count; i++) {
        memory_raw_t *raw = &memory_info->raws[i];
        tty_printf("p.address: %#x, size: %#x, type = %d\n",
            raw->base_l, raw->length_l, raw->type);
        if (raw->type == 1)
            total_size += raw->length_l;
    }
    total_size -= MEMORY_EXT_START;
    total_size &= 0xFFFFF000;
    tty_printf("total available memory size: %#x\n", total_size);
    // 用户内存开始位置
    extern uint32_t memory_start[];
    uint32_t total_start = (uint32_t)&memory_start;
    memory32_info_t memory32_info;
    init_memory32_info(&memory32_info, 
        (uint8_t *)total_start, 
        MEMORY_EXT_START, 
        total_size, 
        PAGE_SIZE);
    // 每页占用1bit, 计算位图大小
    total_start += bitmap_bit_size(total_size / PAGE_SIZE);
    // 忽略偏移量，计算最大内存页的地址
    total_size &= 0xFFFFF000;
    // 初始化内存位图
    init_memory32_info(&memory32_info, 
        (uint8_t *)total_start, 
        MEMORY_EXT_START, 
        total_size, 
        PAGE_SIZE);
    tty_printf("total available memory start: %#x\n", total_start);
    // 开启分页
    enable_paging();
    // 映射内核页
    kernel_pagging();
}