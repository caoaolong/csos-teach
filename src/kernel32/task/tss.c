#include <task/tss.h>
#include <csos/string.h>
#include <csos/memory.h>
#include <csos/syscall.h>
#include <paging.h>
#include <interrupt.h>

static mutex_t task_mutex;
static tss_task_t task_table[OS_TASK_MAX_SIZE];

tss_task_queue_t tss_task_queue;

static uint32_t task_pid = 0;

static uint32_t idle_task_stack[1024];
static void idle_task_entry()
{
    while (TRUE) HLT;
}

static int tss_init(tss_task_t *task, uint32_t flag, uint32_t entry, uint32_t esp)
{
    uint32_t selector = alloc_gdt_table_entry();
    if (selector < 0) return selector;

    tss_t *tss = &task->tss;
    set_gdt_table_entry(selector, (uint32_t)tss, sizeof(tss_t),
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_TYPE_TSS);

    kernel_memset(tss, 0, sizeof(tss_t));

    uint32_t kernel_stack = alloc_page();

    uint32_t uc_selector = KERNEL_CODE_SEG, ud_selector = KERNEL_DATA_SEG;
    if (flag & TASK_LEVEL_USER) {
        uc_selector = tss_task_queue.uc_selector | SEG_ATTR_CPL3;
        ud_selector = tss_task_queue.ud_selector | SEG_ATTR_CPL3;
    }

    tss->eip = entry;
    tss->esp = esp;
    tss->esp0 = kernel_stack + PAGE_SIZE;
    tss->ss = tss->es = tss->ds = tss->fs = tss->gs = ud_selector;
    tss->ss0 = KERNEL_DATA_SEG;
    tss->cs = uc_selector;
    tss->eflags = EFLAGS_DEFAULT | EFLAGS_IF;

    uint32_t pde = memory32_create_pde();
    if (pde == 0) {
        free_gdt_table_entry(selector);
        return -1;
    }
    tss->cr3 = pde;
    task->selector = selector;
    return 0;
}

static task_t *alloc_task()
{
    tss_task_t *task = NULL;
    mutex_lock(&task_mutex);
    for (int i = 0; i < OS_TASK_MAX_SIZE; i++)
    {
        tss_task_t *t = &task_table[i];
        if (t->name[0] == 0) {
            task = t;
            break;
        }
    }
    mutex_unlock(&task_mutex);
    return task;
}

uint32_t tss_task_getpid()
{
    task_t *task = get_running_task();
    return task->pid;
}

void tss_task_queue_init()
{
    kernel_memset(task_table, 0, sizeof(task_table));
    mutex_init(&task_mutex);

    uint32_t ud_selector = alloc_gdt_table_entry();
    set_gdt_table_entry(ud_selector, 0x0, 0xFFFFFFFF,
        SEG_ATTR_P | SEG_ATTR_DPL3 | SEG_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_ATTR_D);
    tss_task_queue.ud_selector = ud_selector;

    uint32_t uc_selector = alloc_gdt_table_entry();
    set_gdt_table_entry(uc_selector, 0x0, 0xFFFFFFFF,
        SEG_ATTR_P | SEG_ATTR_DPL3 | SEG_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_ATTR_D);
    tss_task_queue.uc_selector = uc_selector;

    list_init(&tss_task_queue.ready_list);
    list_init(&tss_task_queue.task_list);
    list_init(&tss_task_queue.sleep_list);
    tss_task_queue.running_task = NULL;
    // 初始化空闲任务
    tss_task_init(&tss_task_queue.idle_task, "idle task", TASK_LEVEL_SYSTEM, (uint32_t)idle_task_entry, (uint32_t)&idle_task_stack[1024]);
}

void default_tss_task_init()
{
    // default task 入口
    void init_task_entry();
    // default task 代码开始结束位置
    extern uint8_t b_init_task[], e_init_task[];
    // 计算需要拷贝的字节数
    uint32_t copy_size = (uint32_t)(e_init_task - b_init_task);
    // 分配空间
    uint32_t alloc_size = PAGE_SIZE * 10;
    // 初始化任务
    uint32_t init_start = (uint32_t)init_task_entry;
    tss_task_init(&tss_task_queue.default_task, "default task", TASK_LEVEL_USER, init_start, init_start + alloc_size);
    write_tr(tss_task_queue.default_task.selector);
    tss_task_queue.running_task = &tss_task_queue.default_task;
    uint32_t pde = tss_task_queue.default_task.tss.cr3;
    set_pde(pde);
    alloc_pages(pde, init_start, alloc_size, PTE_P | PTE_W | PTE_U);
    kernel_memcpy((void *)init_start, (void *)b_init_task, copy_size);
}

tss_task_t *get_default_tss_task()
{
    return &tss_task_queue.default_task;
}

tss_task_t *get_running_tss_task()
{
    return tss_task_queue.running_task;
}

void tss_task_set_ready(tss_task_t *task)
{
    // 防止空闲任务进入运行任务队列
    if (task == &tss_task_queue.idle_task) return;

    task->state = TASK_READY;
    list_insert_back(&tss_task_queue.ready_list, &task->running_node);
}

void tss_task_set_block(tss_task_t *task)
{
    // 防止空闲任务进入运行任务队列
    if (task == &tss_task_queue.idle_task) return;
    
    list_remove(&tss_task_queue.ready_list, &task->running_node);
}

void tss_task_yield()
{
    if (tss_task_queue.ready_list.size > 1)
    {
        tss_task_t *task = get_running_tss_task();
        tss_task_set_block(task);
        tss_task_set_ready(task);
        tss_task_dispatch();
    }
}

int tss_task_fork()
{
    tss_task_t *parent = get_running_task();
    tss_task_t *child = alloc_task();
    if (child == NULL) 
        return -1;
    
    syscall_frame_t *frame = (syscall_frame_t *)(parent->tss.esp0 - sizeof(syscall_frame_t));
    int ret = tss_task_init(child, parent->name, TASK_LEVEL_USER, 
            frame->eip, 
            frame->_esp + sizeof(uint32_t) * SYSCALL_PMC);
    if (ret != 0) {
        tss_task_destroy(child);
        return -1;
    }

    tss_t *tss = &child->tss;
    tss->eax = 0;
    tss->edx = frame->edx;
    tss->ecx = frame->ecx;
    tss->ebx = frame->ebx;
    tss->esi = frame->esi;
    tss->edi = frame->edi;
    tss->ebp = frame->ebp;
    tss->cs = frame->cs;
    tss->ds = frame->ds;
    tss->es = frame->es;
    tss->gs = frame->gs;
    tss->fs = frame->fs;
    tss->eflags = frame->eflags;

    child->parent = parent;

    if ((tss->cr3 = copy_page(parent->tss.cr3)) < 0) {
        tss_task_destroy(child);
        return -1;
    }

    return child->pid;
}

void tss_task_exit(int code)
{
    task_t *task = get_running_task();
    if (task->state == TASK_RUNNING) {
        task->state = TASK_DIED;
        task->exit_code = code;
    }
    list_remove(&tss_task_queue.ready_list, &task->running_node);
    list_remove(&tss_task_queue.task_list, &task->task_node);
    tss_task_destroy(task);
    tss_task_dispatch();
}

// SHELL程序临时存放位置
uint8_t SHELL_TMP[512 * 20];

static uint32_t load_elf_file(tss_task_t *task, const char *name, uint32_t pde)
{
    read_disk(1000, 20, (uint16_t *)SHELL_TMP);
    uint8_t *buffer = SHELL_TMP;
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)buffer;
    if (elf_header->e_ident[0] != 0x7F || 
        elf_header->e_ident[1] != 'E' || 
        elf_header->e_ident[2] != 'L' || 
        elf_header->e_ident[3] != 'F') {
        return -1;
    }

    for (int i = 0; i < elf_header->e_phnum; i++) {
        Elf32_Phdr *phdr = (Elf32_Phdr *)(buffer + elf_header->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        alloc_pages(pde, phdr->p_vaddr, phdr->p_memsz, PTE_P | PTE_W | PTE_U);
        uint32_t vaddr = phdr->p_vaddr;
        uint32_t size = phdr->p_memsz;
        while (size > 0)
        {
            int cz = (size > PAGE_SIZE) ? PAGE_SIZE : size;
            uint32_t paddr = memory32_get_paddr(pde, vaddr);
            kernel_memcpy(buffer, (uint8_t *)paddr, cz);
            vaddr += cz;
            buffer += cz;
            size -= cz;
        }
    }
    return 0;
}

int tss_task_execve(const char *name, const char *args, const char *env)
{
    tss_task_t *task = get_running_task();
    uint32_t origial_pde = task->tss.cr3;
    uint32_t new_pde = memory32_create_pde();
    if (new_pde <= 0) return -1;
    if (load_elf_file(task, name, new_pde) < 0) return -1;
    task->tss.cr3 = new_pde;
    set_pde(new_pde);
    destroy_page(origial_pde);
    return 0;
}

void tss_task_destroy(tss_task_t *task)
{
if (task->selector) {
        free_gdt_table_entry(task->selector);
    }

    if (task->tss.esp0) {
        free_page(task->tss.esp - PAGE_SIZE);
    }

    if (task->tss.cr3) {
        destroy_page(task->tss.cr3);
    }
    kernel_memset(task, 0, sizeof(task_t));
}

void tss_task_ts()
{
    protect_state_t ps = protect_enter();
    list_node_t *node = list_get_first(&tss_task_queue.sleep_list);
    while (node)
    {
        tss_task_t *task = struct_from_field(node, tss_task_t, running_node);
        if (-- task->sleep == 0) {
            tss_task_notify(task);
            tss_task_set_ready(task);
        }
        node = task->running_node.next;
    }

    tss_task_t *task = get_running_tss_task();
    if (-- task->ticks == 0)
    {
        task->ticks = task->slices;
        tss_task_set_block(task);
        tss_task_set_ready(task);
        tss_task_dispatch();
    }
    protect_exit(ps);
}

static void tss_task_set_sleep(tss_task_t *task, uint32_t ticks)
{
    if (ticks == 0) return;

    task->sleep = ticks;
    task->state = TASK_SLEEP;
    list_insert_back(&tss_task_queue.sleep_list, &task->running_node);
}

void tss_task_sleep(uint32_t ms)
{
    tss_task_set_block(tss_task_queue.running_task);
    tss_task_set_sleep(tss_task_queue.running_task, ms);
    tss_task_dispatch();
}

void tss_task_notify(tss_task_t *task)
{
    list_remove(&tss_task_queue.sleep_list, &task->running_node);
}

void tss_task_switch(tss_task_t *from, tss_task_t *to)
{
    far_jump(to->selector, 0);
}

void tss_task_dispatch()
{
    protect_state_t ps = protect_enter();
    tss_task_t *to = &tss_task_queue.idle_task, *from = get_running_tss_task();
    // 判断运行队列是否为空，为空则运行空闲任务
    if (!list_is_empty(&tss_task_queue.ready_list))
    {
        list_node_t *node = list_get_first(&tss_task_queue.ready_list);
        to = struct_from_field(node, tss_task_t, running_node);
    }
    if (to != from)
    {
        tss_task_queue.running_task = to;
        to->state = TASK_RUNNING;
        set_pde(to->tss.cr3);
        tss_task_switch(from, to);
    }
    protect_exit(ps);
}

int tss_task_init(tss_task_t *task, const char *name, uint32_t flag, uint32_t entry, uint32_t esp)
{
    int r = tss_init(task, flag, entry, esp);
    if (r < 0) return r;

    kernel_strcpy(task->name, name);
    task->state = TASK_CREATED;
    list_node_init(&task->task_node);
    list_node_init(&task->running_node);
    // 插入任务队列
    list_insert_front(&tss_task_queue.task_list, &task->task_node);
    // 插入就绪队列
    protect_state_t ps = protect_enter();
    tss_task_set_ready(task);
    protect_exit(ps);
    // 任务时间片初始化
    task->ticks = task->slices = TASK_DEFAULT_TICKS;
    // 延时
    task->sleep = 0;
    task->pid = task_pid++;
    return 0;
}