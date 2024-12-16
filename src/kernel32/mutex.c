#include <csos/mutex.h>
#include <interrupt.h>
#include <task.h>

void mutex_init(mutex_t *mutex)
{
    mutex->locker = 0;
    mutex->owner = NULL;
    list_init(&mutex->wait_list);
}

void mutex_lock(mutex_t *mutex)
{
    protect_state_t ps = protect_enter();
    task_t *running_task = get_running_task();

    if (mutex->locker == 0) {
        mutex->locker ++;
        mutex->owner = running_task;
    } else if (mutex->owner == running_task) {
        mutex->locker ++;
    } else {
        task_set_block(running_task);
        list_insert_back(&mutex->wait_list, &running_task->wait_node);
        task_dispatch();
    }

    protect_exit(ps);
}

void mutex_unlock(mutex_t *mutex)
{
    protect_state_t ps = protect_enter();

    task_t *running_task = get_running_task();

    if (mutex->owner == running_task) {
        if (-- mutex->locker == 0) {
            mutex->owner = NULL;
            if (!list_is_empty(&mutex->wait_list)) {
                list_node_t *node = list_remove_front(&mutex->wait_list);
                task_t *task = struct_from_field(node, task_t, wait_node);
                task_set_ready(task);
                mutex->locker ++;
                mutex->owner = task;
                task_dispatch();
            }
        }
    }

    protect_exit(ps);
}