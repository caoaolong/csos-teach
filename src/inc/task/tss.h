#ifndef CSOS_TASK_TSS_H
#define CSOS_TASK_TSS_H

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

typedef struct tss_task_t {
    task_state_t state;
    char name[TASK_NAME_SIZE];
    // 当前正在运行的任务列表节点
    list_node_t running_node;
    // 所有任务列表节点
    list_node_t task_node;
    // 等待任务列表节点
    list_node_t wait_node;
    tss_t tss;
    uint32_t selector;
    // 任务时间片
    uint32_t ticks;
    uint32_t slices;
    // 延时
    uint32_t sleep;
} tss_task_t;

typedef struct tss_task_queue_t
{
    // 就绪列表
    list_t ready_list;
    // 所有任务列表
    list_t task_list;
    // 延时任务列表
    list_t sleep_list;
    // 默认任务
    tss_task_t default_task;
    // 空闲任务
    tss_task_t idle_task;
    // 当前正在运行的任务
    tss_task_t *running_task;
} tss_task_queue_t;

void tss_task_queue_init();

void default_tss_task_init();

tss_task_t *get_default_tss_task();

tss_task_t *get_running_tss_task();

void tss_task_set_ready(tss_task_t *task);

void tss_task_set_block(tss_task_t *task);

void tss_task_yield();

void tss_task_ts();

void tss_task_sleep(uint32_t ms);

void tss_task_notify(tss_task_t *task);

void tss_task_dispatch();

int tss_task_init(tss_task_t *task, const char *name, uint32_t entry, uint32_t esp);

#endif