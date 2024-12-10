#ifndef CSOS_TASK_H
#define CSOS_TASK_H

#include <kernel.h>
#include <list.h>

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

#define TASK_NAME_SIZE  32

typedef struct tss_task_t {
    enum {
        TASK_CREATED,
        TASK_RUNNING,
        TASK_SLEEP,
        TASK_READY,
        TASK_WAITING
    } state;
    char name[TASK_NAME_SIZE];
    // 当前正在运行的任务列表节点
    list_node_t running_node;
    // 所有任务列表节点
    list_node_t task_node;
    tss_t tss;
    uint32_t selector;
} tss_task_t;

typedef struct task_queue_t
{
    // 就绪列表
    list_t ready_list;
    // 所有任务列表
    list_t task_list;
    // 默认任务
    tss_task_t default_task;
    // 当前正在运行的任务
    tss_task_t *running_task;
} task_queue_t;

typedef struct simple_task_t
{
    uint32_t *stack;
    int selector;
} simple_task_t;

void task_queue_init();

void default_task_init();

tss_task_t *get_default_task();

tss_task_t *get_running_task();

void task_set_ready(tss_task_t *task);

void task_set_block(tss_task_t *task);

void task_yield();

void task_dispatch();

void tss_task_init(tss_task_t *task, const char *name, uint32_t entry, uint32_t esp);

void tss_task_switch(tss_task_t *from, tss_task_t *to);

void simple_task_init(simple_task_t *task, uint32_t entry, uint32_t esp);

void simple_task_switch(simple_task_t *from, simple_task_t *to);

#endif