#include <csos/syscall.h>
#include <interrupt.h>
#include <task.h>
#include <tty.h>
#include <logf.h>

void syscall_tty_logf(char *fmt, int arg)
{
    tty_logf(fmt, arg);
}

static const syscall_handler_t syscall_handler_table[] = {
    [SYS_NR_SLEEP] = (syscall_handler_t)task_sleep,
    [SYS_NR_GETPID] = (syscall_handler_t)task_getpid,
    [SYS_NR_LOGF] = (syscall_handler_t)syscall_tty_logf,
    [SYS_NR_FORK] = (syscall_handler_t)task_fork
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
    tty_logf("task: %s syscall(%d) error!", task->name, frame->id);
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
    tty_logf("task: %s syscall(%d) error!", task->name, id);
}