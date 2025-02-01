#include <kernel.h>
#include <tty.h>
#include <interrupt.h>
#include <logf.h>
#include <csos/time.h>
#include <task.h>
#include <csos/sem.h>
#include <csos/memory.h>
#include <device.h>

void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    // 初始化中断
    interrupt_init();
    // 初始化调试输出
    logf_init();
    // 打开所需设备
    device_open(DEV_TTY, 0, NULL);
    // 初始化时间
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
    // 打印信息
    logf("KL Version: %s; OS Version: %s", KERNEL_VERSION, OP_SYS_VERSION);
    // default任务
    task_t *task = get_running_task();
    task_goto(task);
}