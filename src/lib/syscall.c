#include <csos/syscall.h>
#include <interrupt.h>
#include <task.h>
#include <tty.h>
#include <logf.h>
#include <fs.h>

static const syscall_handler_t syscall_handler_table[] = {
    [SYS_NR_SLEEP]      = (syscall_handler_t)task_sleep,
    [SYS_NR_GETPID]     = (syscall_handler_t)task_getpid,
    [SYS_NR_FORK]       = (syscall_handler_t)task_fork,
    [SYS_NR_YIELD]      = (syscall_handler_t)task_yield,
    [SYS_NR_EXIT]       = (syscall_handler_t)task_exit,
    [SYS_NR_EXECVE]     = (syscall_handler_t)task_execve,
    [SYS_NR_SBRK]       = (syscall_handler_t)task_sbrk,
    [SYS_NR_PRINTF]     = (syscall_handler_t)fs_fwrite,
    [SYS_NR_OPENDIR]    = (syscall_handler_t)fs_opendir,
    [SYS_NR_READDIR]    = (syscall_handler_t)fs_readdir,
    [SYS_NR_CLOSEDIR]   = (syscall_handler_t)fs_closedir,
    [SYS_NR_FOPEN]      = (syscall_handler_t)fs_fopen,
    [SYS_NR_FGETS]      = (syscall_handler_t)fs_fread,
    [SYS_NR_FPUTS]      = (syscall_handler_t)fs_fwrite,
    [SYS_NR_FCLOSE]     = (syscall_handler_t)fs_fclose,
    [SYS_NR_GETCWD]     = (syscall_handler_t)fs_getcwd,
    [SYS_NR_CHDIR]      = (syscall_handler_t)fs_chdir,
    [SYS_NR_MKDIR]      = (syscall_handler_t)fs_mkdir,
    [SYS_NR_RMDIR]      = (syscall_handler_t)fs_rmdir,
    [SYS_NR_LSEEK]      = (syscall_handler_t)fs_lseek,
    [SYS_NR_REMOVE]     = (syscall_handler_t)fs_remove,
    [SYS_NR_GETC]       = (syscall_handler_t)fs_getc,
    [SYS_NR_PUTC]       = (syscall_handler_t)fs_putc,
    [SYS_NR_DUP]        = (syscall_handler_t)fs_dup,
    [SYS_NR_CLEAR]      = (syscall_handler_t)sys_clear
};
// 远调用实现
void syscall(syscall_frame_t *frame)
{
    uint32_t size = sizeof(syscall_handler_table) / sizeof(syscall_handler_table[0]);
    if (frame->id < size)
    {
        syscall_handler_t handler = syscall_handler_table[frame->id];
        if (handler)
        {
            frame->eax = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
            return;
        }
    }
    task_t *task = get_running_task();
    logf("task: %s syscall(%d) error!", task->name, frame->id);
}

// 软中断实现
void handler_syscall(interrupt_frame_t* frame)
{
    uint32_t id = frame->eax;
    uint32_t arg0 = frame->ebx, arg1 = frame->ecx, arg2 = frame->edx, arg3 = frame->esi;
    uint32_t size = sizeof(syscall_handler_table) / sizeof(syscall_handler_table[0]);
    if (id < size)
    {
        syscall_handler_t handler = syscall_handler_table[id];
        if (handler)
        {
            frame->eax = handler(arg0, arg1, arg2, arg3);
            return;
        }
    }
    task_t *task = get_running_task();
    logf("task: %s syscall(%d) error!", task->name, id);
}