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
#define SYS_NR_FPUTS        20
#define SYS_NR_LSEEK        21
#define SYS_NR_REMOVE       22
#define SYS_NR_GETC         23
#define SYS_NR_PUTC         24
#define SYS_NR_DUP          25
#define SYS_NR_CLEAR        26
#define SYS_NR_TCGETATTR    27
#define SYS_NR_TCSETATTR    28
#define SYS_NR_FREE         29
#define SYS_NR_WAIT         30
#define SYS_NR_TEST         31
#define SYS_NR_ARPL         32
#define SYS_NR_ARPC         33
#define SYS_NR_PING         34
#define SYS_NR_IFCONF       35
#define SYS_NR_ENUM_PORT    36
#define SYS_NR_SOCKET       37
#define SYS_NR_CONNECT      38
#define SYS_NR_CLOSE        39

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

static inline int wait(int *stats)
{
    syscall_arg_t wait_arg = { SYS_NR_WAIT, (int)stats, 0, 0, 0 };
    return _syscall(&wait_arg);
}

static inline void exit(int code)
{
    syscall_arg_t exit_arg = { SYS_NR_EXIT, code, 0, 0, 0 };
    _syscall(&exit_arg);
}

static inline int execve(const char *name, char *const *argv, const char *env)
{
    syscall_arg_t execve_arg = { SYS_NR_EXECVE, (uint32_t)name, (uint32_t)argv, (uint32_t)env, 0 };
    return _syscall(&execve_arg);
}

static inline void *malloc(uint32_t size)
{
    syscall_arg_t sbrk_arg = { SYS_NR_SBRK, size, 0, 0, 0 };
    return (void *)_syscall(&sbrk_arg);
}

static inline void free(void *ptr)
{
    syscall_arg_t free_arg = { SYS_NR_FREE, (int)ptr, 0, 0, 0 };
    _syscall(&free_arg);
}

static inline void test()
{
    syscall_arg_t test_arg = { SYS_NR_TEST, 0, 0, 0, 0 };
    _syscall(&test_arg);
}

#endif