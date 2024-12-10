#include <task/simple.h>
#include <csos/string.h>

simple_task_queue_t simple_task_queue;

void simple_task_queue_init()
{
    list_init(&simple_task_queue.ready_list);
    list_init(&simple_task_queue.task_list);
    simple_task_queue.running_task = NULL;
}

void default_simple_task_init()
{
    // 初始化任务
    simple_task_init(&simple_task_queue.default_task, "default task", 0, 0);
    simple_task_queue.running_task = &simple_task_queue.default_task;
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
    task->state = TASK_READY;
    list_insert_back(&simple_task_queue.ready_list, &task->running_node);
}

void simple_task_set_block(simple_task_t *task)
{
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

void simple_task_dispatch()
{
    list_node_t *node = list_get_first(&simple_task_queue.ready_list);
    simple_task_t *to = struct_from_field(node, simple_task_t, running_node);
    simple_task_t *from = get_running_simple_task();
    if (to != from)
    {
        simple_task_queue.running_task = to;
        to->state = TASK_RUNNING;
        simple_task_switch(from, to);
    }
}

void simple_task_init(simple_task_t *task, const char *name, uint32_t entry, uint32_t esp)
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
    kernel_strcpy(task->name, name);
    task->state = TASK_CREATED;
    list_node_init(&task->task_node);
    list_node_init(&task->running_node);
    // 插入任务队列
    list_insert_front(&simple_task_queue.task_list, &task->task_node);
    // 插入就绪队列
    simple_task_set_ready(task);
}

extern void simple_switch(uint32_t **from, uint32_t *to);

void simple_task_switch(simple_task_t *from, simple_task_t *to)
{
    simple_switch((uint32_t **)&from->stack, to->stack);
}