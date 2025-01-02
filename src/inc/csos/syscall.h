#ifndef CSOS_SYSCALL_H
#define CSOS_SYSCALL_H

#include <kernel.h>

#define SYSCALL_PMC         5
#define SYS_NR_SLEEP        1
#define SYS_NR_GETPID       2
#define SYS_NR_LOGF         3

void syscall_handler();

typedef struct syscall_frame_t
{
    // 手动压入
    uint32_t eflags;
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    // 自动压入
    uint32_t eip, cs;
    uint32_t id, arg0, arg1, arg2, arg3;
    uint32_t _esp, ss;
} syscall_frame_t;

typedef struct syscall_arg_t
{
    // 系统调用号
    uint32_t id;
    // 参数
    int arg0, arg1, arg2, arg3;
} syscall_arg_t;

static inline int _syscall(syscall_arg_t *arg)
{
    uint32_t addr[] = { 0, SYSCALL_GATE_SEG | 0 };
    uint32_t ret;
    __asm__ volatile("push %[arg3]\r\n"
            "push %[arg2]\r\n"
            "push %[arg1]\r\n"
            "push %[arg0]\r\n"
            "push %[id]\r\n"
            "lcalll *(%[a])\r\n"
            :"=a"(ret)
            :[arg3]"r"(arg->arg3), [arg2]"r"(arg->arg2), [arg1]"r"(arg->arg1), [arg0]"r"(arg->arg0), [id]"r"(arg->id), [a]"r"(addr));
    return ret;
}

static inline void sys_sleep(uint32_t ms)
{
    syscall_arg_t sleep_arg = { SYS_NR_SLEEP, ms, 0, 0, 0 };
    _syscall(&sleep_arg);
}

static inline uint32_t sys_getpid()
{
    syscall_arg_t getpid_arg = { SYS_NR_GETPID, 0, 0, 0, 0 };
    return _syscall(&getpid_arg);
}

static inline void sys_logf(const char *fmt, int arg)
{
    syscall_arg_t printf_arg = { SYS_NR_LOGF, (uint32_t)fmt, arg, 0, 0 };
    _syscall(&printf_arg);
}

#endif