#include <kernel.h>
#include <tty.h>
#include <interrupt.h>
#include <logf.h>
#include <csos/time.h>
#include <task.h>
#include <csos/sem.h>
#include <csos/memory.h>
#include <device.h>
#include <disk.h>
#include <fs.h>
#include <pci.h>
#include <pci/e1000.h>
#include <netx.h>

void csos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    // 初始化中断
    interrupt_init();
    // 初始化调试输出
    logf_init();
    // 初始化随机数
    if (!(get_rdseed_support() & 0x1)) {
        logf("not support real random");
    }
    // 初始化时间
    time_init(OS_TZ);
    // 初始化内存
    memory_init(mem_info);
    // GDT重载
    gdt32_init((gdt_table_t*)gdt_info);
    // 检测磁盘
    disk_init();
    // 文件系统初始化
    fs_init();
    // PCI初始化
    pci_init();
    // 网卡初始化
    e1000_init();
    // 网络配置
    net_init();
    // 初始化任务队列
    task_queue_init();
    // 初始化任务
    default_task_init();
    // 打印信息
    logf("KL Version: %s; OS Version: %s", KERNEL_VERSION, OP_SYS_VERSION);
    // default任务
    task_t *task = get_running_task();
    task_goto(task);
}