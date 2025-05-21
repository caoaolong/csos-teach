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
    // uint32_t ud_selector = alloc_gdt_table_entry();
    // set_gdt_table_entry(ud_selector, 0x0, 0xFFFFFFFF,
    //     SEG_ATTR_P | SEG_ATTR_DPL3 | SEG_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_ATTR_D);
    // simple_task_queue.ud_selector = ud_selector;

    // uint32_t uc_selector = alloc_gdt_table_entry();
    // set_gdt_table_entry(uc_selector, 0x0, 0xFFFFFFFF,
    //     SEG_ATTR_P | SEG_ATTR_DPL3 | SEG_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_ATTR_D);
    // simple_task_queue.uc_selector = uc_selector;

    list_init(&simple_task_queue.ready_list);
    list_init(&simple_task_queue.task_list);
    list_init(&simple_task_queue.sleep_list);
    simple_task_queue.running_task = NULL;
    // 初始化空闲任务
    simple_task_init(&simple_task_queue.idle_task, "idle task", TASK_LEVEL_SYSTEM, (uint32_t)idle_task_entry, (uint32_t)&idle_task_stack[1024]);
}

void default_simple_task_init()
{
    // init task 入口
    void init_task_entry();
    // init task 代码开始结束位置
    extern uint8_t b_init_task[], e_init_task[];
    // 计算需要拷贝的字节数
    uint32_t copy_size = (uint32_t)(e_init_task - b_init_task);
    // 分配空间
    uint32_t alloc_size = PAGE_SIZE * 10;
    // 初始化任务
    uint32_t init_start = (uint32_t)init_task_entry;
    simple_task_init(&simple_task_queue.default_task, "default task", TASK_LEVEL_USER, init_start, init_start + alloc_size);
    simple_task_queue.running_task = &simple_task_queue.default_task;
    uint32_t pde = simple_task_queue.default_task.pde;
    set_pde(pde);
    alloc_task_pages(pde, init_start, alloc_size, PTE_P | PTE_W | PTE_U);
    kernel_memcpy((void *)init_start, (void *)b_init_task, copy_size);
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

int simple_task_init(simple_task_t *task, const char *name, uint32_t flag, uint32_t entry, uint32_t esp)
{
    uint32_t pde = memory32_create_pde();
    if (pde == 0) return -1;

    // uint32_t kernel_stack = alloc_page();

    // uint32_t uc_selector = KERNEL_CODE_SEG, ud_selector = KERNEL_DATA_SEG;
    // if (flag & TASK_LEVEL_USER) {
    //     uc_selector = simple_task_queue.uc_selector | SEG_ATTR_CPL3;
    //     ud_selector = simple_task_queue.ud_selector | SEG_ATTR_CPL3;
    // }
    task->pde = pde;
    task->entry = entry;
    if (flag & TASK_LEVEL_USER) {
        task->ss = USER_DATA_SEG | SEG_ATTR_CPL3;
        task->cs = USER_CODE_SEG | SEG_ATTR_CPL3;
    } else {
        task->ss = KERNEL_DATA_SEG;
        task->cs = KERNEL_CODE_SEG;
    }
    task->eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    uint32_t *pesp = (uint32_t *)esp;
    if (pesp) {
        // ss
        *(--pesp) = task->ss;
        // esp
        *(--pesp) = esp;
        // eflags
        *(--pesp) = task->eflags;
        // cs
        *(--pesp) = task->cs;
        // eip
        *(--pesp) = entry;
        // error code
        *(--pesp) = 0;
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
    protect_state_t ps = protect_enter();
    simple_task_set_ready(task);
    protect_exit(ps);
    // 任务时间片初始化
    task->ticks = task->slices = TASK_DEFAULT_TICKS;
    // 延时
    task->sleep = 0;
    return 0;
}