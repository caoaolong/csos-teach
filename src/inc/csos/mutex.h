#ifndef CSOS_MUTEX_H
#define CSOS_MUTEX_H

#include <list.h>
#include <task/tss.h>
#include <task/simple.h>

typedef struct mutex_t
{
    #ifdef TASK_SIMPLE
        // 锁的拥有者
        simple_task_t *owner;
    #endif

    #ifdef TASK_TSS
        // 锁的拥有者
        tss_task_t *owner;
    #endif

    // 重入次数
    uint32_t locker;
    // 等待队列
    list_t wait_list;
} mutex_t;

void mutex_init(mutex_t *mutex);

void mutex_lock(mutex_t *mutex);

void mutex_unlock(mutex_t *mutex);

#endif