#ifndef CSOS_MEMORY_H
#define CSOS_MEMORY_H

#include <kernel.h>
#include <mutex.h>
#include <bitmap.h>

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

void memory_init(memory_info_t *memory_info);

#endif