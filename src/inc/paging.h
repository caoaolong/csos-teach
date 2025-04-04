#ifndef CSOS_PAGING_H
#define CSOS_PAGING_H

#include <kernel.h>

#define PAGE_SIZE           0x1000
#define KERNEL_PAGE_COUNT   1024
#define PDE_COUNT           1024
#define PTE_COUNT           1024

#define PTE_P               (1 << 0)
#define PTE_W               (1 << 1)
#define PTE_U               (1 << 2)

#define PDE_P               (1 << 0)
#define PDE_W               (1 << 1)
#define PDE_U               (1 << 2)

#define PDE_INDEX(index)    ((index) >> 22)
#define PTE_INDEX(index)    ((index) >> 12 & 0x3FF)
#define PDE_ADDRESS(p)      ((p)->index << 12)
#define PTE_ADDRESS(p)      ((p)->index << 12)
#define PTE_PERM(pte)       ((pte)->v & 0x3FF)

typedef union pde_t
{
    uint32_t v;
    struct {
        uint32_t present: 1;
        uint32_t write: 1;
        uint32_t user: 1;
        uint32_t pwt: 1;
        uint32_t pcd: 1;
        uint32_t accessed: 1;
        uint32_t : 1;
        uint32_t ps: 1;
        uint32_t : 4;
        uint32_t index: 20;
    };
} pde_t;

typedef union pte_t
{
    uint32_t v;
    struct {
        uint32_t present: 1;
        uint32_t write: 1;
        uint32_t user: 1;
        uint32_t pwt: 1;
        uint32_t pcd: 1;
        uint32_t accessed: 1;
        uint32_t dirty: 1;
        uint32_t pat: 1;
        uint32_t global: 1;
        uint32_t : 3;
        uint32_t index: 20;
    };
} pte_t;

static inline void set_pde(int index)
{
    write_cr3((uint32_t)index);
}

#endif