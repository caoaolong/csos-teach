#ifndef CSOS_MEMORY_H
#define CSOS_MEMORY_H

#include <kernel.h>
#include <csos/mutex.h>
#include <bitmap.h>

// 可用内存开始位置
#define MEMORY_EXT_START    0x100000
// 可用内存结束位置(32M)
#define MEMORY_EXT_LIMIT    0x2000000

#define MEMORY_EBDA_START   0x80000

typedef struct memory32_info_t
{
    // 内存分配锁
    mutex_t mutex;    
    // 内存起始地址
    uint32_t start;
    // 总字节数
    uint32_t size;
    // 每页的字节数
    uint32_t page_size;
    // 内存位图
    bitmap_t bitmap;
} memory32_info_t;

typedef struct memory32_map_t
{
    // 虚拟起始地址
    void *vstart;
    // 虚拟结束地址
    void *vlimit;
    // 物理起始地址
    void *pstart;
    // 权限
    uint32_t perm;
} memory32_map_t;

uint32_t memory32_create_pde();

void memory_init(memory_info_t *memory_info);

#endif