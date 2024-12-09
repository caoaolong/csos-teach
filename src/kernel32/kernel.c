#include <kernel.h>
#include <tty.h>
#include <interrupt.h>
#include <logf.h>
#include <csos/time.h>
#include <task.h>
#include <list.h>

// static tss_task_t main_task, child_task;
static simple_task_t main_task, child_task;
static uint32_t child_task_stack[1024];

void child_task_entry()
{
    int counter = 0;
    while (TRUE) {
        tty_logf("child task: %d", counter++);
        // tss_task_switch(&child_task, &main_task);
        simple_task_switch(&child_task, &main_task);
    }
}

void main_task_entry()
{
    int counter = 0;
    while (TRUE) {
        tty_logf("main task: %d", counter++);
        // tss_task_switch(&main_task, &child_task);
        simple_task_switch(&main_task, &child_task);
    }
}

void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    interrupt_init();
    tty_logf_init();
    tty_logf("KL Version: %s; OS Version: %s", KERNEL_VERSION, OP_SYS_VERSION);
    time_init(OS_TZ);
    // GDT重载
    gdt32_init((gdt_table_t*)gdt_info);
    // 开启中断
    // sti();

    test_list();
}