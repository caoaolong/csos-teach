#ifndef CSOS_TASK_H
#define CSOS_TASK_H

#include <task/simple.h>
#include <task/tss.h>
/*---------------任务模式------------------*/
#define TASK_SIMPLE
/*----------------------------------------*/

#ifdef TASK_SIMPLE

#define task_t              simple_task_t
#define task_yield          simple_task_yield
#define task_sleep          simple_task_sleep
#define task_notify         simple_task_notify
#define task_init           simple_task_init
#define task_queue_init     simple_task_queue_init
#define default_task_init   default_simple_task_init
#define get_default_task    get_default_simple_task
#define get_running_task    get_running_simple_task
#define task_ts             simple_task_ts
#define task_dispatch       simple_task_dispatch
#define task_set_ready      simple_task_set_ready
#define task_set_block      simple_task_set_block
#define task_goto(task)     __asm__ volatile("jmp *%[ip]"::[ip]"r"((task)->entry));

#endif

#ifdef TASK_TSS

#define task_t              tss_task_t
#define task_yield          tss_task_yield
#define task_sleep          tss_task_sleep
#define task_notify         tss_task_notify
#define task_init           tss_task_init
#define task_queue_init     tss_task_queue_init
#define default_task_init   default_tss_task_init
#define get_default_task    get_default_tss_task
#define get_running_task    get_running_tss_task
#define task_ts             tss_task_ts
#define task_dispatch       tss_task_dispatch
#define task_set_ready      tss_task_set_ready
#define task_set_block      tss_task_set_block
#define task_goto(task)     __asm__ volatile("jmp *%[ip]"::[ip]"r"((task)->tss.eip));
#endif

#endif