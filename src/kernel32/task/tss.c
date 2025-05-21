#include <task/tss.h>
#include <csos/string.h>
#include <csos/memory.h>
#include <csos/syscall.h>
#include <csos/stdlib.h>
#include <paging.h>
#include <interrupt.h>
#include <fs.h>
#include <logf.h>

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
    tss_task_queue.default_task.bheap = (uint32_t)e_init_task;
    tss_task_queue.default_task.eheap = (uint32_t)e_init_task;
    write_tr(tss_task_queue.default_task.selector);
    tss_task_queue.running_task = &tss_task_queue.default_task;
    uint32_t pde = tss_task_queue.default_task.tss.cr3;
    set_pde(pde);
    alloc_task_pages(pde, init_start, alloc_size, PTE_P | PTE_W | PTE_U);
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

int tss_task_wait(int *code)
{
    task_t *task = get_running_task();
    // 关闭文件描述符
    for (int i = 0; i < TASK_FT_SIZE; i++) {
        FILE *pf = task->ftb[i];
        if (pf) fs_fclose(i);
    }
    task->state = TASK_DYING;
    *code = task->exit_code;
    return 0;
}

void tss_task_exit(int code)
{
    task_t *task = get_running_task();
    if (task->state == TASK_DYING) {
        task->state = TASK_DIED;
        task->exit_code = code;
    }
    list_remove(&tss_task_queue.ready_list, &task->running_node);
    list_remove(&tss_task_queue.task_list, &task->task_node);
    tss_task_destroy(task);
    tss_task_dispatch();
}

static uint8_t SHELL_TMP[200 * 512];

static uint32_t load_elf_file(task_t *task, const char *name, uint32_t pde)
{
    Elf32_Ehdr elf_hdr;
    Elf32_Phdr elf_phdr;
    read_disk(1000, 200, (uint16_t *)SHELL_TMP);
    uint8_t *buffer = SHELL_TMP;
    kernel_memcpy(&elf_hdr, buffer, sizeof(Elf32_Ehdr));
    buffer += sizeof(Elf32_Ehdr);
    if (elf_hdr.e_ident[0] != 0x7F || elf_hdr.e_ident[1] != 'E' ||elf_hdr.e_ident[2] != 'L' ||elf_hdr.e_ident[3] != 'F')
        return 0;
    uint32_t e_phoff = elf_hdr.e_phoff;
    for (int i = 0; i < elf_hdr.e_phnum; i++) {
        buffer = SHELL_TMP + e_phoff;
        kernel_memcpy(&elf_phdr, buffer, sizeof(Elf32_Phdr));
        buffer += sizeof(Elf32_Phdr);
        if ((elf_phdr.p_type != 1) || (elf_phdr.p_vaddr < VM_TASK_BASE))
            continue;
        int err = alloc_task_pages(pde, elf_phdr.p_vaddr, elf_phdr.p_memsz, PTE_P | PTE_U | PTE_W);
        if (err < 0) return -1;
        buffer = SHELL_TMP + elf_phdr.p_offset;
        uint32_t vaddr = elf_phdr.p_vaddr;
        uint32_t size = elf_phdr.p_filesz;
        while (size > 0)
        {
            int cs = (size > PAGE_SIZE) ? PAGE_SIZE : size;
            uint32_t paddr = get_paddr(pde, vaddr);
            kernel_memcpy((void *)paddr, buffer, cs);
            buffer += cs;
            size -= cs;
            vaddr += cs;
        }
        task->eheap = task->bheap = elf_phdr.p_vaddr + elf_phdr.p_memsz;
    }
    return elf_hdr.e_entry;
}

static int copy_args(uint32_t pde, char *dst, char *argv[], int argc)
{
    task_args_t task_args;
    task_args.argc = argc;
    task_args.argv = (char **)(dst + sizeof(task_args_t));

    char *dst_arg = dst + sizeof(task_args_t) + sizeof(char *) * argc;
    char **dst_arg_tb = (char **)get_paddr(pde, (uint32_t)(dst + sizeof(task_args_t)));
    for (int i = 0; i < argc; i++)
    {
        char *from = argv[i];
        int len = kernel_strlen(from) + 1;
        int err = memory32_copy_page_data((uint32_t)dst_arg, pde, (uint32_t)from, len);
        if (err < 0) return -1;
        dst_arg_tb[i] = dst_arg;
        dst_arg += len;
    }
    memory32_copy_page_data((uint32_t)dst, pde, (uint32_t)&task_args, sizeof(task_args));
}

int tss_task_execve(char *name, char *argv[], char *env[])
{
    mutex_lock(&task_mutex);
    tss_task_t *task = get_running_task();
    kernel_strncpy(task->name, get_file_name(name), TASK_NAME_SIZE);
    uint32_t old_pde = task->tss.cr3;
    uint32_t new_pde = memory32_create_pde();
    if (new_pde <= 0) return -1;
    uint32_t entry = load_elf_file(task, name, new_pde);
    if (entry == 0) {
        task->tss.cr3 = old_pde;
        set_pde(old_pde);
        destroy_pde(old_pde);
        mutex_unlock(&task_mutex);
        return -1;
    }
    uint32_t stack_top = VM_SHELL_STACK - VM_SHELL_ARGS_SIZE;
    if (alloc_task_pages(new_pde, VM_SHELL_STACK - VM_SHELL_STACK_SIZE, VM_SHELL_STACK_SIZE, PTE_U | PTE_W | PTE_P) < 0) {
        task->tss.cr3 = old_pde;
        set_pde(old_pde);
        destroy_pde(old_pde);
        mutex_unlock(&task_mutex);
        return -1;
    }
    int argc = strings_count(argv);
    if (copy_args(new_pde, (char *)stack_top, argv, argc) < 0) {
        task->tss.cr3 = old_pde;
        set_pde(old_pde);
        destroy_pde(old_pde);
        mutex_unlock(&task_mutex);
        return -1;
    }
    syscall_frame_t *frame = (syscall_frame_t *)(task->tss.esp0 - sizeof(syscall_frame_t));
    frame->eip = entry;
    frame->eax = frame->ebx = frame->ecx = frame->edx = 0;
    frame->edi = frame->esi = frame->ebp = 0;
    frame->eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    frame->_esp = stack_top - sizeof(uint32_t) * SYSCALL_PMC;

    task->tss.cr3 = new_pde;
    set_pde(new_pde);
    destroy_pde(old_pde);

    // 初始化堆内存链表
    task->heap = (list_t *)task->bheap;
    list_init(task->heap);
    task->eheap += sizeof(list_t);

    mutex_unlock(&task_mutex);
    return 0;
}

uint8_t *tss_task_sbrk(uint32_t size)
{
    tss_task_t *task = get_running_task();
    uint8_t *peheap = (uint8_t*)task->eheap;
    if (size == 0) return peheap;
    // 内存开始位置
    uint32_t start = task->eheap;
    // 创建内存块
    block_t *pb = (block_t *)start;
    pb->p = TRUE;
    pb->magic = 0xE;
    pb->size = size;
    list_insert_back(task->heap, &pb->node);
    size += sizeof(block_t);
    peheap = (uint8_t *)(start + sizeof(block_t));
    // 内存结束位置
    uint32_t stop = start + size;
    int start_offset = start % PAGE_SIZE;
    if (start_offset) {
        if (start_offset + size <= PAGE_SIZE) {
            task->eheap = stop;
            return peheap;
        } else {
            uint32_t cs = PAGE_SIZE - start_offset;
            start += cs;
            size -= cs;
        }
    }

    if (size) {
        uint32_t cs = stop - start;
        if (alloc_task_pages(task->tss.cr3, start, cs, PTE_P | PTE_U | PTE_W) < 0) {
            return (uint8_t*)NULL;
        }
    }

    task->eheap = stop;
    return peheap;
}

void tss_task_free(void *ptr)
{
    block_t *pb = ptr - sizeof(block_t);
    if (pb->p) {
        pb->p = FALSE;
    }
    uint32_t size = pb->size;
    // 整理内存
    task_t *task = get_running_task();
    list_node_t *pnode = list_get_last(task->heap);
    if (&pb->node == pnode) {
        list_remove(task->heap, pnode);
        // 检查是否有可释放的内存页
        uint32_t pbegin = (uint32_t)pb;
        uint32_t pend = pbegin + size + sizeof(block_t);
        if ((pbegin & 0xFFFFF000) < (pend & 0xFFFFF000)) {
            free_page(pend & 0xFFFFF000);
        }
    }
    task->eheap = (uint32_t)pb;
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
        destroy_pde(task->tss.cr3);
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
    task->bheap = task->eheap = 0;
    task->wd.offset = task->wd.sector = -1;
    // 文件表
    kernel_memset(task->ftb, 0, sizeof(task->ftb));
    return 0;
}

FILE *tss_task_file(int fd)
{
    if (fd >= 0 && fd < TASK_FT_SIZE) {
        return get_running_task()->ftb[fd];
    }
    return NULL;
}

int tss_task_alloc_fd(FILE *file)
{
    task_t *task = get_running_task();
    for (int i = 0; i < TASK_FT_SIZE; i++) {
        FILE *fp = task->ftb[i];
        if (fp == NULL) {
            task->ftb[i] = file;
            return i;
        }
    }
    return -1;
}

void tss_task_free_fd(int fd)
{
    if (fd >= 0 && fd < TASK_FT_SIZE) {
        task_t *task = get_running_task();
        task->ftb[fd] = NULL;
    }
}