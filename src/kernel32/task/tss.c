#include <task/tss.h>
#include <csos/string.h>
#include <interrupt.h>
#include <timer.h>

tss_task_queue_t tss_task_queue;

static uint32_t idle_task_stack[1024];
static void idle_task_entry()
{
    while (TRUE) HLT;
}

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

void tss_task_queue_init()
{
    list_init(&tss_task_queue.ready_list);
    list_init(&tss_task_queue.task_list);
    list_init(&tss_task_queue.sleep_list);
    tss_task_queue.running_task = NULL;
    // 初始化空闲任务
    tss_task_init(&tss_task_queue.idle_task, "idle task", (uint32_t)idle_task_entry, (uint32_t)&idle_task_stack[1024]);
}

void default_tss_task_init()
{
    // 初始化任务
    tss_task_init(&tss_task_queue.default_task, "default task", 0, 0);
    write_tr(tss_task_queue.default_task.selector);
    tss_task_queue.running_task = &tss_task_queue.default_task;
}

tss_task_t *get_default_tss_task()
{
    return &tss_task_queue.default_task;
}

tss_task_t *get_running_tss_task()
{
    return tss_task_queue.running_task;
}

void tss_task_set_ready(tss_task_t *task)
{
    // 防止空闲任务进入运行任务队列
    if (task == &tss_task_queue.idle_task) return;

    task->state = TASK_READY;
    list_insert_back(&tss_task_queue.ready_list, &task->running_node);
}

void tss_task_set_block(tss_task_t *task)
{
    // 防止空闲任务进入运行任务队列
    if (task == &tss_task_queue.idle_task) return;
    
    list_remove(&tss_task_queue.ready_list, &task->running_node);
}

void tss_task_yield()
{
    if (tss_task_queue.ready_list.size > 1)
    {
        tss_task_t *task = get_running_tss_task();
        tss_task_set_block(task);
        tss_task_set_ready(task);
        tss_task_dispatch();
    }
}

void tss_task_ts()
{
    protect_state_t ps = protect_enter();
    list_node_t *node = list_get_first(&tss_task_queue.sleep_list);
    while (node)
    {
        tss_task_t *task = struct_from_field(node, tss_task_t, running_node);
        if (-- task->sleep == 0) {
            tss_task_notify(task);
            tss_task_set_ready(task);
        }
        node = task->running_node.next;
    }

    tss_task_t *task = get_running_tss_task();
    if (-- task->ticks == 0)
    {
        task->ticks = task->slices;
        tss_task_set_block(task);
        tss_task_set_ready(task);
        tss_task_dispatch();
    }
    protect_exit(ps);
}

static void tss_task_set_sleep(tss_task_t *task, uint32_t ticks)
{
    if (ticks == 0) return;

    task->sleep = ticks;
    task->state = TASK_SLEEP;
    list_insert_back(&tss_task_queue.sleep_list, &task->running_node);
}

void tss_task_sleep(uint32_t ms)
{
    tss_task_set_block(tss_task_queue.running_task);
    tss_task_set_sleep(tss_task_queue.running_task, ms);
    tss_task_dispatch();
}

void tss_task_notify(tss_task_t *task)
{
    list_remove(&tss_task_queue.sleep_list, &task->running_node);
}

void tss_task_dispatch()
{
    protect_state_t ps = protect_enter();
    tss_task_t *to = &tss_task_queue.idle_task, *from = get_running_tss_task();
    // 判断运行队列是否为空，为空则运行空闲任务
    if (!list_is_empty(&tss_task_queue.ready_list))
    {
        list_node_t *node = list_get_first(&tss_task_queue.ready_list);
        to = struct_from_field(node, tss_task_t, running_node);
    }
    if (to != from)
    {
        tss_task_queue.running_task = to;
        to->state = TASK_RUNNING;
        tss_task_switch(from, to);
    }
    protect_exit(ps);
}

void tss_task_init(tss_task_t *task, const char *name, uint32_t entry, uint32_t esp)
{
    tss_init(task, entry, esp);
    kernel_strcpy(task->name, name);
    task->state = TASK_CREATED;
    list_node_init(&task->task_node);
    list_node_init(&task->running_node);
    // 插入任务队列
    list_insert_front(&tss_task_queue.task_list, &task->task_node);
    // 插入就绪队列
    tss_task_set_ready(task);
    // 任务时间片初始化
    task->ticks = task->slices = TASK_DEFAULT_TICKS;
    // 延时
    task->sleep = 0;
}

void tss_task_switch(tss_task_t *from, tss_task_t *to)
{
    far_jump(to->selector, 0);
}