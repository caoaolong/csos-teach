#include <memory.h>

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
    uint32_t addr = 0x1000;
    init_memory32_info(&memory32_info, bits, 0x1000, 64 * 4096, 4096);
    for (int i = 0; i < 32; i++)
    {
        addr = memory32_alloc_page(&memory32_info, 2);
        logf("+ Alloc page(start = 0x%x)", addr);
    }

    addr = 0x1000;
    for (int i = 0; i < 32; i++)
    {
        memory32_free_page(&memory32_info, addr, 2);
        logf("- Free page(start = 0x%x)", addr);
        addr += 8192;   
    }
}