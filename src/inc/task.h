#ifndef CSOS_TASK_H
#define CSOS_TASK_H

#include <kernel.h>
#include <list.h>

#define TASK_NAME_SIZE  32

typedef enum {
    TASK_CREATED,
    TASK_RUNNING,
    TASK_SLEEP,
    TASK_READY,
    TASK_WAITING
} task_state_t;

#endif