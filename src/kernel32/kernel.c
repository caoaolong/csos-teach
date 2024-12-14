#include <kernel.h>
#include <tty.h>
#include <interrupt.h>
#include <logf.h>
#include <csos/time.h>
#include <task/simple.h>
#include <task/tss.h>

static uint32_t test_task_stack[1024];

#ifdef TASK_SIMPLE
    static simple_task_t test_task;
#endif

#ifdef TASK_TSS
    static tss_task_t test_task;
#endif

void test () {
    uint32_t counter = 0;
    while (TRUE)
    {
        #ifdef TASK_SIMPLE
            simple_task_t *task = get_running_simple_task();
        #endif

        #ifdef TASK_TSS
            tss_task_t *task = get_running_tss_task();
        #endif
        tty_logf("%s : %d", task->name, counter++);
        #ifdef TASK_SIMPLE
            simple_task_sleep(1000);
        #endif
        #ifdef TASK_TSS
            tss_task_sleep(1000);
        #endif
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
    // 初始化任务队列
    #ifdef TASK_SIMPLE
        simple_task_queue_init();
    #endif
    
    #ifdef TASK_TSS
        tss_task_queue_init();
    #endif

    // 初始化default任务
    #ifdef TASK_SIMPLE
        simple_task_init(&test_task, "test task", (uint32_t)test, (uint32_t)&test_task_stack[1024]);
        default_simple_task_init();
    #endif

    #ifdef TASK_TSS
        tss_task_init(&test_task, "test task", (uint32_t)test, (uint32_t)&test_task_stack[1024]);
        default_tss_task_init();
    #endif

    // 开启中断
    sti();
    // main任务
    int counter = 0;
    while (TRUE) {
        #ifdef TASK_SIMPLE
            simple_task_t *task = get_running_simple_task();
        #endif

        #ifdef TASK_TSS
            tss_task_t *task = get_running_tss_task();
        #endif
        tty_logf("%s : %d", task->name, counter++);
        #ifdef TASK_SIMPLE
            simple_task_sleep(1000);
        #endif
        #ifdef TASK_TSS
            tss_task_sleep(1000);
        #endif
    }
}