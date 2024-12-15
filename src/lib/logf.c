#include <logf.h>
#include <kernel.h>
#include <interrupt.h>
#include <csos/stdarg.h>
#include <csos/string.h>
#include <csos/stdio.h>
#include <csos/mutex.h>

void tty_logf_init()
{
    // 关闭中断
    outb(COM1_PORT + 1, 0x00);
    // 启用DLAB
    outb(COM1_PORT + 3, 0x80);
    // 设置波特率除数（115200 / 0x03）
    // 低8位
    outb(COM1_PORT + 0, 0x03);
    // 高8位
    outb(COM1_PORT + 1, 0x00);
    // 设置线路协议
    outb(COM1_PORT + 3, 0x03);
    // 启用 FIFO，清除它们，阈值为 14 字节
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0B);
}

extern mutex_t mutex;

void tty_logf(const char * fmt, ...)
{
    char buffer[1024];
    va_list args;
    kernel_memset(buffer, '\0', sizeof(buffer));
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    
    mutex_lock(&mutex);
    const char *p = buffer;
    while (*p != 0)
    {
        while (inb(COM1_PORT + 5) & (1 << 6) == 0);
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
    mutex_unlock(&mutex);
}