#include <csos/sem.h>
#include <interrupt.h>
#include <task/simple.h>
#include <task/tss.h>
#include <csos/mutex.h>

extern mutex_t mutex;

void sem_init(sem_t *sem, uint32_t count)
{
    sem->counter = count;
    list_init(&sem->wait_list);
}

void sem_wait(sem_t *sem)
{
    mutex_lock(&mutex);
    if (sem->counter > 0)
    {
        sem->counter --;
    } else {
        #ifdef TASK_SIMPLE
        simple_task_t *task = get_running_simple_task();
        simple_task_set_block(task);
        list_insert_back(&sem->wait_list, &task->wait_node);
        simple_task_dispatch();
        #endif

        #ifdef TASK_TSS
        tss_task_t *task = get_running_tss_task();
        tss_task_set_block(task);
        list_insert_back(&sem->wait_list, &task->wait_node);
        tss_task_dispatch();
        #endif
    }
    mutex_unlock(&mutex);
}

void sem_notify(sem_t *sem)
{
    mutex_lock(&mutex);
    if (!list_is_empty(&sem->wait_list))
    {
        list_node_t *node = list_remove_front(&sem->wait_list);
        #ifdef TASK_SIMPLE
        simple_task_t *task = struct_from_field(node, simple_task_t, wait_node);
        simple_task_set_ready(task);
        simple_task_dispatch();
        #endif

        #ifdef TASK_TSS
        tss_task_t *task = struct_from_field(node, tss_task_t, wait_node);
        tss_task_set_ready(task);
        tss_task_dispatch();
        #endif
    } else {
        sem->counter ++;
    }
    mutex_unlock(&mutex);
}

uint32_t sem_count(sem_t *sem)
{
    mutex_lock(&mutex);
    uint32_t count = sem->counter;
    mutex_unlock(&mutex);
    return count;
}