#include <csos/sem.h>
#include <interrupt.h>
#include <task.h>
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
        task_t *task = get_running_task();
        task_set_block(task);
        list_insert_back(&sem->wait_list, &task->wait_node);
        task_dispatch();
    }
    mutex_unlock(&mutex);
}

void sem_notify(sem_t *sem)
{
    mutex_lock(&mutex);
    if (!list_is_empty(&sem->wait_list))
    {
        list_node_t *node = list_remove_front(&sem->wait_list);
        task_t *task = struct_from_field(node, task_t, wait_node);
        task_set_ready(task);
        task_dispatch();
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