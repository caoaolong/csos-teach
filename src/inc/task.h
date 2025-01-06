#ifndef CSOS_TASK_H
#define CSOS_TASK_H

#include <task/simple.h>
#include <task/tss.h>
/*---------------任务模式------------------*/
/* 注: 由于调试不方便，从系统调用功能开始不再兼容TASK_SIMPLE */
#define TASK_TSS
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
#define task_goto(task)     __asm__ volatile("push %[ss]\r\n"                       \
                                    "push %[esp]\r\n"                               \
                                    "push %[eflags]\r\n"                            \
                                    "push %[cs]\r\n"                                \
                                    "push %[eip]\r\n"                               \
                                    "iret\r\n"                                      \
                                    ::[ss]"r"((task)->ss), [esp]"r"((task)->stack), \
                                    [eflags]"r"((task)->eflags), [cs]"r"((task)->cs), [eip]"r"((task)->entry));

#endif

#ifdef TASK_TSS

#define task_t              tss_task_t
#define task_yield          tss_task_yield
#define task_sleep          tss_task_sleep
#define task_getpid         tss_task_getpid
#define task_fork           tss_task_fork
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
#define task_goto(task)     tss_t *tss = &(task)->tss;                      \
                            __asm__ volatile("push %[ss]\r\n"               \
                                    "push %[esp]\r\n"                       \
                                    "push %[eflags]\r\n"                    \
                                    "push %[cs]\r\n"                        \
                                    "push %[eip]\r\n"                       \
                                    "iret\r\n"                              \
                                    ::[ss]"r"(tss->ss), [esp]"r"(tss->esp), \
                                    [eflags]"r"(tss->eflags), [cs]"r"(tss->cs), [eip]"r"(tss->eip));
#endif

#endif