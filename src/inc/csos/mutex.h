#ifndef CSOS_MUTEX_H
#define CSOS_MUTEX_H

#include <list.h>
#include <task.h>

typedef struct mutex_t
{
    // 锁的拥有者
    task_t *owner;
    // 重入次数
    uint32_t locker;
    // 等待队列
    list_t wait_list;
} mutex_t;

void mutex_init(mutex_t *mutex);

void mutex_lock(mutex_t *mutex);

void mutex_unlock(mutex_t *mutex);

#endif