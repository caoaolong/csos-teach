#include <csos/memory.h>
#include <tty.h>
#include <paging.h>
#include <csos/string.h>

static memory32_info_t memory32_info;
static pde_t kernel_pde[KERNEL_PAGE_COUNT] __attribute__((aligned(PAGE_SIZE)));

static void init_memory32_info(memory32_info_t *memory32_info, uint8_t *bits, 
    uint32_t start, uint32_t size, uint32_t page_size)
{
    mutex_init(&memory32_info->mutex);
    memory32_info->size = size;
    memory32_info->page_size = page_size;
    memory32_info->start = start;
    bitmap_init(&memory32_info->bitmap, bits, size / page_size, FLASE);
}

static uint32_t memory32_alloc_page(memory32_info_t *memory32_info, uint32_t count)
{
    mutex_lock(&memory32_info->mutex);
    uint32_t addr = 0;
    int page_index = bitmap_alloc_bits(&memory32_info->bitmap, FLASE, count);
    if (page_index >= 0)
    {
        addr = memory32_info->start + page_index * memory32_info->page_size;
    }
    mutex_unlock(&memory32_info->mutex);
    return addr;
}

static void memory32_free_page(memory32_info_t *memory32_info, uint32_t addr, uint32_t count)
{
    mutex_lock(&memory32_info->mutex);
    uint32_t page_index = (addr  - memory32_info->start) / memory32_info->page_size;
    bitmap_set_bits(&memory32_info->bitmap, page_index, count, FLASE);
    mutex_unlock(&memory32_info->mutex);
}

pte_t *find_pte(pde_t *page_dir, uint32_t vaddr, BOOL alloc)
{
    pte_t *pte;
    pde_t *pde = page_dir + PDE_INDEX(vaddr);
    if (pde->present) {
        pte = (pte_t *)PDE_ADDRESS(pde);
    } else {
        if (!alloc) {
            return NULL;
        }
        uint32_t addr = memory32_alloc_page(&memory32_info, 1);
        if (addr == 0) {
            return NULL;
        }
        pde->v = addr | PDE_P | PDE_W | PDE_U;
        pte = (pte_t *)addr;
        kernel_memset(pte, 0, PAGE_SIZE);
    }

    return pte + PTE_INDEX(vaddr);
}

static int memory32_create_map(pde_t *pde, uint32_t vaddr, uint32_t paddr, uint32_t count, uint32_t perm)
{
    for (uint32_t i = 0; i < count; i++)
    {
        pte_t *pte = find_pte(pde, vaddr, TRUE);
        if (pte == NULL) return -1;
        pte->v = paddr | perm | PTE_P;
        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }
}

static inline uint32_t floor_page(uint32_t value)
{
    return value & ~(PAGE_SIZE - 1);
}

static inline uint32_t ceil_page(uint32_t value)
{
    return (value + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

static void kernel_pagging()
{
    extern uint32_t b_text[], e_text[], b_data[], e_data[], b_kernel[], e_kernel[];

    static memory32_map_t kernel_map[] = {
        { b_kernel, b_text, b_kernel, PTE_W },
        { b_text, e_text, b_text, 0 },
        { b_data, (void *)MEMORY_EBDA_START, b_data, PTE_W },
        { (void *)MEMORY_EXT_START, (void *)MEMORY_EXT_LIMIT, (void *)MEMORY_EXT_START, PTE_W }
    };
    uint32_t count = sizeof(kernel_map) / sizeof(memory32_map_t);
    for (int i = 0; i < count; i++)
    {
        memory32_map_t *map = &kernel_map[i];
        uint32_t vstart = floor_page((uint32_t)map->vstart);
        uint32_t vlimit = ceil_page((uint32_t)map->vlimit);
        uint32_t paddr = floor_page((uint32_t)map->pstart);
        uint32_t page_count = (vlimit - vstart) / PAGE_SIZE;

        memory32_create_map(kernel_pde, vstart, paddr, page_count, map->perm);
    }
}

uint32_t memory32_create_pde()
{
    pde_t *pde = (pde_t *)memory32_alloc_page(&memory32_info, 1);
    if (pde == 0) return 0;
    kernel_memset((void *)pde, 0, PAGE_SIZE);
    uint32_t user_pde_start = PDE_INDEX(MEMORY_EXT_LIMIT);
    // 映射内核页表
    for (int i = 0; i < user_pde_start; i++)
    {
        pde[i].v = kernel_pde[i].v;
    }
    return (uint32_t)pde;
}

static int memory32_alloc_task_pages(uint32_t pde, uint32_t index, uint32_t size, uint32_t perm)
{
    uint32_t current_index = index;
    int count = ceil_page(size) / PAGE_SIZE;

    for (int i = 0; i < count; i++)
    {
        uint32_t paddr = memory32_alloc_page(&memory32_info, 1);   
        if (paddr == 0) {
            tty_printf("memory allocate failed.");
            return -1;
        }

        int error = memory32_create_map((pde_t *)pde, current_index, paddr, 1, perm);
        if (error < 0) {
            tty_printf("memory create failed.");
            return -1;
        }

        current_index += PAGE_SIZE;
    }

    return 0;
}

uint32_t alloc_page()
{
    return memory32_alloc_page(&memory32_info, 1);
}

void free_page(uint32_t addr)
{
    extern uint8_t b_init_task[];
    if (addr < (uint32_t)b_init_task) {
        memory32_free_page(&memory32_info, addr, 1);
    } else {
        pte_t *pte = NULL;
        #ifdef TASK_TSS
            pte = find_pte((pde_t *)get_running_task()->tss.cr3, addr, FLASE);
        #endif

        #ifdef TASK_SIMPLE
            pte = find_pte((pde_t *)get_running_task()->pde, addr, FLASE);
        #endif
        memory32_free_page(&memory32_info, PTE_INDEX((uint32_t)pte), 1);
        pte->v = 0;
    }
}

int alloc_pages(uint32_t index, uint32_t size, uint32_t perm)
{
    #ifdef TASK_SIMPLE
        return memory32_alloc_task_pages(get_running_task()->pde, index, size, perm);
    #endif

    #ifdef TASK_TSS
        return memory32_alloc_task_pages(get_running_task()->tss.cr3, index, size, perm);
    #endif
}

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
    total_size = floor_page(total_size);
    tty_printf("total available memory size: %#x\n", total_size);
    // 用户内存开始位置
    extern uint32_t memory_start[];
    uint32_t total_start = (uint32_t)&memory_start;
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
    // 映射内核页
    kernel_pagging();
    // 设置内核页
    set_pde((uint32_t)kernel_pde);
}