#include <csos/syscall.h>
#include <task.h>
#include <logf.h>

typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

static const syscall_handler_t syscall_handler_table[] = {
    [SYS_NR_SLEEP] = (syscall_handler_t)task_sleep
};

void syscall(syscall_frame_t *frame)
{
    uint32_t size = sizeof(syscall_handler_table) / sizeof(syscall_handler_table[0]);
    if (frame->id < size)
    {
        syscall_handler_t handler = syscall_handler_table[frame->id];
        if (handler)
        {
            int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
            return;
        }
    }
    task_t *task = get_running_task();
    tty_logf("task: %s syscall(%d) error!", task->name, frame->id);
}