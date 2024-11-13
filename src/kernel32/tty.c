#include <string.h>
#include <tty.h>
#include <stdio.h>
#include <kernel.h>
#include <stdarg.h>

#define CRT_ADDR_REG        0x3D4
#define CRT_DATA_REG        0x3D5

#define CRT_START_ADDR_H    0xC
#define CRT_START_ADDR_L    0xD
#define CRT_CURSOR_H        0xE
#define CRT_CURSOR_L        0xF

// 显卡文本模式内存映射的起始地址
#define MEM_BASE    0xB8000
// 显卡文本模式内存大小
#define MEM_SIZE    0x4000
// 显卡文本模式内存映射的结束地址
#define MEM_END     (MEM_BASE + MEM_SIZE)
// 显卡文本模式宽（字符数）
#define WIDTH       80
// 显卡文本模式高（行数）
#define HEIGHT      25
// 每行所占内存
#define ROW_SIZE    (WIDTH * 2)
// 整个屏幕所占内存
#define SCR_SIZE    (ROW_SIZE * HEIGHT)

#define ASCII_NUL   0x00
#define ASCII_ENQ   0x05
#define ASCII_BEL   0x07
#define ASCII_BS    0x08
#define ASCII_HT    0x09
#define ASCII_LF    0x0A
#define ASCII_VT    0x0B
#define ASCII_FF    0x0C
#define ASCII_CR    0x0D
#define ASCII_DEL   0x0F

// 屏幕当前位置
static uint32_t screen;
// 光标内存位置
static uint32_t pos;
// 光标当前位置
static uint16_t x = 0, y = 0;
// 字符样式
static uint8_t attr = 7;
// 空格
static uint16_t erase = 0x0720;

static void get_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    screen = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    screen |= inb(CRT_DATA_REG);

    screen <<= 1;
    screen += MEM_BASE;
}

static void set_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xFF);
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xFF);
}

static void get_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    pos = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG);

    get_screen();
    pos <<= 1;
    pos += MEM_BASE;

    uint32_t delta = (pos - screen) >> 1;
    x = delta % WIDTH;
    y = delta / WIDTH;
}

static void set_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0XFF);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0XFF);
}

void tty_clear()
{
    screen = MEM_BASE;
    set_screen();
    pos = MEM_BASE;
    set_cursor();

    uint16_t *ptr = (uint16_t*)MEM_BASE;
    while (ptr < (uint16_t*)MEM_END) {
        *ptr++ = erase;
    }
}

static void com_bs()
{
    if (x){
        x --;
        pos -= 2;
        *(uint16_t*)pos = erase;
    }
}

static void com_del()
{
    *(uint16_t*)pos = erase;
}

static void com_cr()
{
    pos -= (x << 1);
    x = 0;
}

static void scroll_up()
{
    if (screen + SCR_SIZE + ROW_SIZE >= MEM_END) {
        kernel_memcpy((uint32_t*)MEM_BASE, (uint32_t*)screen, SCR_SIZE);
        pos -= (screen - MEM_BASE);
        screen = MEM_BASE;
    }
    uint32_t *ptr = (uint32_t*)(screen + SCR_SIZE);
    for (int i = 0; i < WIDTH; ++i) {
        *ptr++ = erase;
    }
    screen += ROW_SIZE;
    pos += ROW_SIZE;
    set_screen();
}

static void com_lf()
{
    if (y + 1 < HEIGHT) {
        y ++;
        pos += ROW_SIZE;
        return;
    }
    scroll_up();
}

uint32_t tty_write(char *buf, uint32_t count)
{
    char c;
    char *ptr = (char *)pos;
    uint32_t nr = 0;
    while (nr ++ < count)
    {
        c = *buf++;
        switch (c) {
            case ASCII_NUL: break;
            case ASCII_BS: com_bs(); break;
            case ASCII_LF: com_lf(); com_cr(); break;
            case ASCII_FF: com_lf(); break;
            case ASCII_CR: com_cr(); break;
            case ASCII_DEL: com_del(); break;
            default:
                if (x >= WIDTH) {
                    x -= WIDTH;
                    pos -= ROW_SIZE;
                    com_lf();
                }
                *ptr = c; *(ptr + 1) = (char)attr;
                ptr += 2; pos += 2; x++;
                break;
        }
    }
    set_cursor();
    return nr;
}

void tty_init()
{
    tty_clear();
}

#define BUFFER_SIZE 1024
static char buf[BUFFER_SIZE];

int tty_printf(const char *fmt, ...)
{
    kernel_memset(buf, 0, BUFFER_SIZE);
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    tty_write(buf, (uint32_t)i);
    return i;
}