#include <kernel.h>
#include <tty.h>
#include <interrupt.h>
#include <logf.h>
#include <csos/time.h>
#include <task.h>
#include <csos/sem.h>
#include <csos/memory.h>

void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    interrupt_init();
    tty_logf_init();
    tty_logf("KL Version: %s; OS Version: %s", KERNEL_VERSION, OP_SYS_VERSION);
    time_init(OS_TZ);
    // 初始化内存
    memory_init(mem_info);
    // GDT重载
    gdt32_init((gdt_table_t*)gdt_info);
    // 初始化任务队列
    task_queue_init();
    // 初始化任务
    default_task_init();
    // 开启中断
    // sti();
    // default任务
    task_t *task = get_running_task();
    task_goto(task);
}