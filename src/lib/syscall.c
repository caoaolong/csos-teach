#include <csos/syscall.h>
#include <interrupt.h>
#include <task.h>
#include <tty.h>
#include <logf.h>
#include <fs.h>
#include <netx.h>

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
    [SYS_NR_CLEAR]      = (syscall_handler_t)sys_clear,
    [SYS_NR_TCGETATTR]  = (syscall_handler_t)tty_tcgetattr,
    [SYS_NR_TCSETATTR]  = (syscall_handler_t)tty_tcsetattr,
    [SYS_NR_FREE]       = (syscall_handler_t)task_free,
    [SYS_NR_WAIT]       = (syscall_handler_t)task_wait,
    [SYS_NR_TEST]       = (syscall_handler_t)sys_test,
    [SYS_NR_ARPL]       = (syscall_handler_t)sys_arpl,
    [SYS_NR_ARPC]       = (syscall_handler_t)sys_arpc,
    [SYS_NR_PING]       = (syscall_handler_t)sys_ping,
    [SYS_NR_IFCONF]     = (syscall_handler_t)sys_ifconf,
    [SYS_NR_ENUM_PORT]  = (syscall_handler_t)sys_enum_port,
    [SYS_NR_SOCKET]     = (syscall_handler_t)sys_socket,
    [SYS_NR_CONNECT]    = (syscall_handler_t)sys_connect,
    [SYS_NR_CLOSE]      = (syscall_handler_t)sys_close,
    [SYS_NR_BIND]       = (syscall_handler_t)sys_bind,
    [SYS_NR_LISTEN]     = (syscall_handler_t)sys_listen,
    [SYS_NR_ACCEPT]     = (syscall_handler_t)sys_accept,
    [SYS_NR_SEND]       = (syscall_handler_t)sys_send,
    [SYS_NR_READ]       = (syscall_handler_t)sys_read,
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