#ifndef CSOS_SEM_H
#define CSOS_SEM_H

#include <list.h>

typedef struct sem_t
{
    int counter;
    list_t wait_list;
} sem_t;

void sem_init();

void sem_wait(sem_t *sem);

void sem_notify(sem_t *sem);

uint32_t sem_count(sem_t *sem);

#endif