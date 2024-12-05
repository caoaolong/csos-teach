#include <task.h>
#include <kernel.h>
#include <csos/string.h>

static void tss_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    uint32_t selector = alloc_gdt_table_entry();
    if (selector < 0) return;

    tss_t *tss = &task->tss;
    set_gdt_table_entry(selector, (uint32_t)tss, sizeof(tss_t),
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_TYPE_TSS);

    kernel_memset(tss, 0, sizeof(tss_t));
    tss->eip = entry;
    tss->esp = tss->esp0 = entry;
    tss->es = tss->ds =  tss->fs =  tss->gs = tss->ss = tss->ss0 = KERNEL_DATA_SEG;
    tss->cs = KERNEL_CODE_SEG;
    tss->eflags = EFLAGS_DEFAULT | EFLAGS_IF;

    task->selector = selector;
}

void tss_task_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    tss_init(task, entry, esp);
}

void tss_task_switch(tss_task_t *from, tss_task_t *to)
{
    far_jump(to->selector, 0);
}