#ifndef CSOS_SYSCALL_H
#define CSOS_SYSCALL_H

#include <kernel.h>

#define SYSCALL_PMC         5
#define SYS_NR_SLEEP        1

void syscall_handler();

typedef struct syscall_arg_t
{
    // 系统调用号
    uint32_t id;
    // 参数
    int arg0, arg1, arg2, arg3;
} syscall_arg_t;

static inline void _syscall(syscall_arg_t *arg)
{
    uint32_t addr[] = { 0, SYSCALL_GATE_SEG | 0 };
    __asm__ volatile("lcalll *(%[a])"::[a]"r"(addr));
}

static inline void sys_sleep(uint32_t ms)
{
    syscall_arg_t sleep_arg = { SYS_NR_SLEEP, ms, 0, 0, 0 };
    _syscall(&sleep_arg);
}

#endif