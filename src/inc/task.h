#ifndef CSOS_TASK_H
#define CSOS_TASK_H

#include <kernel.h>

typedef struct tss_t
{
    uint32_t pre_link;
    uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint32_t iomap;
} tss_t;

typedef struct tss_task_t {
    tss_t tss;
} tss_task_t;

void tss_task_init(tss_task_t *task, uint32_t entry, uint32_t esp);

#endif