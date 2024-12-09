#ifndef CSOS_TASK_SIMPLE_H
#define CSOS_TASK_SIMPLE_H

#include <task.h>

typedef struct simple_task_t
{
    uint32_t *stack;
    task_state_t state;
    char name[TASK_NAME_SIZE];
    // 当前正在运行的任务列表节点
    list_node_t running_node;
    // 所有任务列表节点
    list_node_t task_node;
    // 任务时间片
    uint32_t ticks;
    uint32_t slices;
} simple_task_t;

typedef struct simple_task_queue_t
{
    // 就绪列表
    list_t ready_list;
    // 所有任务列表
    list_t task_list;
    // 默认任务
    simple_task_t default_task;
    // 当前正在运行的任务
    simple_task_t *running_task;
} simple_task_queue_t;

void simple_task_queue_init();

void default_simple_task_init();

simple_task_t *get_default_simple_task();

simple_task_t *get_running_simple_task();

void simple_task_set_ready(simple_task_t *task);

void simple_task_set_block(simple_task_t *task);

void simple_task_yield();

void simple_task_ts();

void simple_task_dispatch();

void simple_task_init(simple_task_t *task, const char *name, uint32_t entry, uint32_t esp);

void simple_task_switch(simple_task_t *from, simple_task_t *to);

#endif