#include <kernel.h>
#include <tty.h>
#include <interrupt.h>
#include <logf.h>
#include <csos/time.h>
#include <task.h>
#include <csos/sem.h>
#include <bitmap.h>

static uint32_t test_task_stack[1024];
static task_t test_task;

void test () {
    uint32_t counter = 0;
    while (TRUE)
    {
        task_t *task = get_running_task();
        tty_logf("%s : %d", task->name, counter++);
        task_yield();
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
    // 测试位图
    uint8_t bits[8];
    bitmap_t bm;
    bitmap_init(&bm, bits, 64, FLASE);
    bitmap_alloc_bits(&bm, FLASE, 10);
    bitmap_set_bits(&bm, 30, 4, TRUE);
    BOOL val = bitmap_get_bit(&bm, 4);
    // 初始化任务队列
    task_queue_init();
    // 初始化任务
    task_init(&test_task, "test task", (uint32_t)test, (uint32_t)&test_task_stack[1024]);
    default_task_init();
    // 开启中断
    sti();
    // main任务
    int counter = 0;
    while (TRUE) {
        task_t *task = get_running_task();
        tty_logf("%s : %d", task->name, counter++);
        task_yield();
    }
}