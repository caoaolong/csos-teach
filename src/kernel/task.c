#include <task.h>
#include <string.h>

static void tss_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.eip = entry;
    task->tss.esp = task->tss.esp0 = entry;
    task->tss.es = task->tss.ds =  task->tss.fs =  task->tss.gs = task->tss.ss = task->tss.ss0 = KERNEL_DATA_SEG;
    task->tss.cs = KERNEL_CODE_SEG;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;
}

void tss_task_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    tss_init(task, entry, esp);
}