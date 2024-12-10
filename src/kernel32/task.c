#include <task.h>
#include <kernel.h>
#include <csos/string.h>

task_queue_t task_queue;

static void tss_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    uint32_t selector = alloc_gdt_table_entry();
    if (selector < 0) return;

    tss_t *tss = &task->tss;
    set_gdt_table_entry(selector, (uint32_t)tss, sizeof(tss_t),
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_TYPE_TSS);

    kernel_memset(tss, 0, sizeof(tss_t));
    tss->eip = entry;
    tss->esp = tss->esp0 = esp;
    tss->es = tss->ds =  tss->fs =  tss->gs = tss->ss = tss->ss0 = KERNEL_DATA_SEG;
    tss->cs = KERNEL_CODE_SEG;
    tss->eflags = EFLAGS_DEFAULT | EFLAGS_IF;

    task->selector = selector;
}

void task_queue_init()
{
    list_init(&task_queue.ready_list);
    list_init(&task_queue.task_list);
    task_queue.running_task = NULL;
}

void default_task_init()
{
    // 初始化任务
    tss_task_init(&task_queue.default_task, "default task", 0, 0);
    write_tr(task_queue.default_task.selector);
    task_queue.running_task = &task_queue.default_task;
}

tss_task_t *get_default_task()
{
    return &task_queue.default_task;
}

tss_task_t *get_running_task()
{
    return task_queue.running_task;
}

void task_set_ready(tss_task_t *task)
{
    task->state = TASK_READY;
    list_insert_back(&task_queue.ready_list, &task->running_node);
}

void task_set_block(tss_task_t *task)
{
    list_remove(&task_queue.ready_list, &task->running_node);
}

void task_yield()
{
    if (task_queue.ready_list.size > 1)
    {
        tss_task_t *task = get_running_task();
        task_set_block(task);
        task_set_ready(task);
        task_dispatch();
    }
}

void task_dispatch()
{
    list_node_t *node = list_get_first(&task_queue.ready_list);
    tss_task_t *to = struct_from_field(node, tss_task_t, running_node);
    tss_task_t *from = get_running_task();
    if (to != from)
    {
        task_queue.running_task = to;
        to->state = TASK_RUNNING;
        tss_task_switch(from, to);
    }
}

void tss_task_init(tss_task_t *task, const char *name, uint32_t entry, uint32_t esp)
{
    tss_init(task, entry, esp);
    kernel_strcpy(task->name, name);
    task->state = TASK_CREATED;
    list_node_init(&task->task_node);
    list_node_init(&task->running_node);
    // 插入任务队列
    list_insert_front(&task_queue.task_list, &task->task_node);
    // 插入就绪队列
    task_set_ready(task);
}

void simple_task_init(simple_task_t *task, uint32_t entry, uint32_t esp)
{
    uint32_t *pesp = (uint32_t *)esp;
    if (pesp) {
        *(--pesp) = entry;
        // ebp
        *(--pesp) = 0;
        // ebx
        *(--pesp) = 1;
        // esi
        *(--pesp) = 2;
        // edi
        *(--pesp) = 3;
        task->stack = pesp;
    }
}

void tss_task_switch(tss_task_t *from, tss_task_t *to)
{
    far_jump(to->selector, 0);
}

extern void simple_switch(uint32_t **from, uint32_t *to);

void simple_task_switch(simple_task_t *from, simple_task_t *to)
{
    simple_switch(&from->stack, to->stack);
}