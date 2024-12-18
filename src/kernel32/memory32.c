#include <csos/memory.h>
#include <tty.h>

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

void memory_init(memory_info_t *memory_info)
{
    memory32_info_t memory32_info;
    uint8_t bits[8];
    uint32_t addr = 0x1000, ps = 4096, as = 8;
    // 创建一个64个bit的位图, 起始地址为0x1000, 每个bit代表一页内存, 一页内存大小为4096bytes
    init_memory32_info(&memory32_info, bits, addr, 64 * ps, ps);
    tty_color_set(COLOR_LIGHT_RED);
    for (int i = 0; i < 8; i++)
    {
        // 每次申请两页内存
        addr = memory32_alloc_page(&memory32_info, as);
        tty_printf("+ Alloc page(start = %#x)\n", addr);
    }
    addr = 0x1000;
    tty_color_set(COLOR_LIGHT_GREEN);
    for (int i = 0; i < 8; i++)
    {
        // 每次释放两页内存
        memory32_free_page(&memory32_info, addr, as);
        tty_printf("- Free page(start = %#x)\n", addr);
        addr += as * ps;
    }
    tty_color_reset();
}