#ifndef CSOS_SYSCALL_H
#define CSOS_SYSCALL_H

#include <kernel.h>
#include <csos/stdarg.h>
#include <csos/stdio.h>

#define SYSCALL_PMC         5
#define SYS_NR_SLEEP        1
#define SYS_NR_GETPID       2
// #define SYS_NR_LOGF         3
#define SYS_NR_FORK         4
#define SYS_NR_YIELD        5
#define SYS_NR_EXIT         6
#define SYS_NR_EXECVE       7
#define SYS_NR_SBRK         8
#define SYS_NR_PRINTF       9
#define SYS_NR_OPENDIR      10
#define SYS_NR_READDIR      11
#define SYS_NR_CLOSEDIR     12
#define SYS_NR_FOPEN        13
#define SYS_NR_FGETS        14
#define SYS_NR_FCLOSE       15
#define SYS_NR_CHDIR        16
#define SYS_NR_GETCWD       17
#define SYS_NR_MKDIR        18
#define SYS_NR_RMDIR        19

#define SYSCALL_LCALL

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

typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

static inline int _syscall(syscall_arg_t *arg)
{
    uint32_t addr[] = { 0, SYSCALL_GATE_SEG | 0 };
    uint32_t ret;
    #ifdef SYSCALL_LCALL
    __asm__ volatile("push %[arg3]\r\n"
            "push %[arg2]\r\n"
            "push %[arg1]\r\n"
            "push %[arg0]\r\n"
            "push %[id]\r\n"
            "lcalll *(%[a])\r\n"
            :"=a"(ret)
            :[arg3]"r"(arg->arg3), [arg2]"r"(arg->arg2), [arg1]"r"(arg->arg1), [arg0]"r"(arg->arg0), [id]"r"(arg->id), [a]"r"(addr));
    #endif

    #ifdef SYSCALL_INT
    __asm__ volatile("int $0x80\r\n"
            :"=a"(ret)
            :"S"(arg->arg3), "d"(arg->arg2), "c"(arg->arg1), "b"(arg->arg0), "a"(arg->id));
    #endif
    return ret;
}

static inline void sleep(uint32_t ms)
{
    syscall_arg_t sleep_arg = { SYS_NR_SLEEP, ms, 0, 0, 0 };
    _syscall(&sleep_arg);
}

static inline uint32_t getpid()
{
    syscall_arg_t getpid_arg = { SYS_NR_GETPID, 0, 0, 0, 0 };
    return _syscall(&getpid_arg);
}

// static inline void logf(const char *fmt, int arg)
// {
//     syscall_arg_t printf_arg = { SYS_NR_LOGF, (uint32_t)fmt, arg, 0, 0 };
//     _syscall(&printf_arg);
// }

static inline int fork()
{
    syscall_arg_t fork_arg = { SYS_NR_FORK, 0, 0, 0, 0 };
    return _syscall(&fork_arg);
}

static inline int yield()
{
    syscall_arg_t yield_arg = { SYS_NR_YIELD, 0, 0, 0, 0 };
    return _syscall(&yield_arg);
}

static inline int exit(int code)
{
    syscall_arg_t exit_arg = { SYS_NR_EXIT, code, 0, 0, 0 };
    return _syscall(&exit_arg);
}

static inline int execve(const char *name, char *const *argv, const char *env)
{
    syscall_arg_t execve_arg = { SYS_NR_EXECVE, (uint32_t)name, (uint32_t)argv, (uint32_t)env, 0 };
    return _syscall(&execve_arg);
}

static inline int malloc(uint32_t size)
{
    syscall_arg_t sbrk_arg = { SYS_NR_SBRK, size, 0, 0, 0 };
    return _syscall(&sbrk_arg);
}

static inline int printf(const char *fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buffer, fmt, args);
    va_end(args);
    syscall_arg_t printf_arg = { SYS_NR_PRINTF, (uint32_t)buffer, i, 0, 0 };
    return _syscall(&printf_arg);
}

#endif