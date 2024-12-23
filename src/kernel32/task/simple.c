#include <task/simple.h>
#include <csos/memory.h>
#include <csos/string.h>
#include <paging.h>
#include <interrupt.h>

simple_task_queue_t simple_task_queue;

static uint32_t idle_task_stack[1024];
static void idle_task_entry()
{
    while (TRUE) HLT;
}

void simple_task_queue_init()
{
    list_init(&simple_task_queue.ready_list);
    list_init(&simple_task_queue.task_list);
    list_init(&simple_task_queue.sleep_list);
    simple_task_queue.running_task = NULL;
    // 初始化空闲任务
    simple_task_init(&simple_task_queue.idle_task, "idle task", (uint32_t)idle_task_entry, (uint32_t)&idle_task_stack[1024]);
}

extern void default_task_entry();

void default_simple_task_init()
{
    // 初始化任务
    simple_task_init(&simple_task_queue.default_task, "default task", (uint32_t)default_task_entry, 0);
    simple_task_queue.running_task = &simple_task_queue.default_task;
    set_pde(simple_task_queue.default_task.pde);
}

simple_task_t *get_default_simple_task()
{
    return &simple_task_queue.default_task;
}

simple_task_t *get_running_simple_task()
{
    return simple_task_queue.running_task;
}

void simple_task_set_ready(simple_task_t *task)
{
    // 防止空闲任务进入运行任务队列
    if (task == &simple_task_queue.idle_task) return;

    task->state = TASK_READY;
    list_insert_back(&simple_task_queue.ready_list, &task->running_node);
}

void simple_task_set_block(simple_task_t *task)
{
    // 防止空闲任务进入运行任务队列
    if (task == &simple_task_queue.idle_task) return;

    list_remove(&simple_task_queue.ready_list, &task->running_node);
}

void simple_task_yield()
{
    if (simple_task_queue.ready_list.size > 1)
    {
        simple_task_t *task = get_running_simple_task();
        simple_task_set_block(task);
        simple_task_set_ready(task);
        simple_task_dispatch();
    }
}

void simple_task_ts()
{
    protect_state_t ps = protect_enter();
    list_node_t *node = list_get_first(&simple_task_queue.sleep_list);
    while (node)
    {
        simple_task_t *task = struct_from_field(node, simple_task_t, running_node);
        if (-- task->sleep == 0) {
            simple_task_notify(task);
            simple_task_set_ready(task);
        }
        node = task->running_node.next;
    }

    simple_task_t *task = get_running_simple_task();
    if (-- task->ticks == 0)
    {
        task->ticks = task->slices;
        simple_task_set_block(task);
        simple_task_set_ready(task);
        simple_task_dispatch();
    }
    protect_exit(ps);
}

static void simple_task_set_sleep(simple_task_t *task, uint32_t ticks)
{
    if (ticks == 0) return;

    task->sleep = ticks;
    task->state = TASK_SLEEP;
    list_insert_back(&simple_task_queue.sleep_list, &task->running_node);
}

void simple_task_sleep(uint32_t ms)
{
    simple_task_set_block(simple_task_queue.running_task);
    simple_task_set_sleep(simple_task_queue.running_task, ms);
    simple_task_dispatch();
}

void simple_task_notify(simple_task_t *task)
{
    list_remove(&simple_task_queue.sleep_list, &task->running_node);
}

extern void simple_switch(uint32_t **from, uint32_t *to);

void simple_task_switch(simple_task_t *from, simple_task_t *to)
{
    set_pde(to->pde);
    simple_switch((uint32_t **)&from->stack, to->stack);
}

void simple_task_dispatch()
{
    protect_state_t ps = protect_enter();
    simple_task_t *to = &simple_task_queue.idle_task, *from = get_running_simple_task();
    // 判断运行队列是否为空，为空则运行空闲任务
    if (!list_is_empty(&simple_task_queue.ready_list))
    {
        list_node_t *node = list_get_first(&simple_task_queue.ready_list);
        to = struct_from_field(node, simple_task_t, running_node);
    }
    if (to != from)
    {
        simple_task_queue.running_task = to;
        to->state = TASK_RUNNING;
        set_pde(to->pde);
        simple_task_switch(from, to);
    }
    protect_exit(ps);
}

int simple_task_init(simple_task_t *task, const char *name, uint32_t entry, uint32_t esp)
{
    uint32_t pde = memory32_create_pde();
    if (pde == 0) return -1;
    task->pde = pde;
    task->entry = entry;
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
        // eflags
        *(--pesp) = EFLAGS_DEFAULT | EFLAGS_IF;
        task->stack = pesp;
    }
    kernel_strcpy(task->name, name);
    task->state = TASK_CREATED;
    list_node_init(&task->task_node);
    list_node_init(&task->running_node);
    // 插入任务队列
    list_insert_front(&simple_task_queue.task_list, &task->task_node);
    // 插入就绪队列
    protect_state_t ps = protect_enter();
    simple_task_set_ready(task);
    protect_exit(ps);
    // 任务时间片初始化
    task->ticks = task->slices = TASK_DEFAULT_TICKS;
    // 延时
    task->sleep = 0;
    return 0;
}