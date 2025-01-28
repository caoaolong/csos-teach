#include <csos/string.h>
#include <tty.h>
#include <interrupt.h>
#include <csos/stdio.h>
#include <kernel.h>
#include <csos/stdarg.h>
#include <csos/mutex.h>

mutex_t mutex;

static dev_terminal_t terminals[TTY_DEV_NR];

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

void tty_clear(dev_terminal_t *term)
{
    int size = term->columns * term->rows;
    for (int i = 0; i < size; i++) {
        tty_char_t *tc = term->base + i;
        tc->c = ' ';
        tc->bg = term->bg;
        tc->fg = term->fg;
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

static void com_cr(dev_terminal_t *term)
{
    term->cc = 0;
}

static void scroll_up(dev_terminal_t *term, int lines)
{
    // 向上滚动n行
    int total = term->columns * term->rows;
    int size = term->columns * lines;
    tty_char_t *dst = term->base;
    tty_char_t *src = term->base + size;
    kernel_memcpy((char*)dst, (char*)src, (total - size) * 2);
    // 清空最后n行
    tty_char_t *ptr = term->base + (total - size);
    for (int i = 0; i < size; i++) {
        (ptr + i)->c = ' ';
    }
}

static void com_lf(dev_terminal_t *term)
{
    if (term->cr + 1 < term->rows) {
        term->cr++;
        return;
    }
    scroll_up(term, 1);
}

void tty_color_set(uint8_t color)
{
    attr = color;
}

void tty_color_reset()
{
    attr = COLOR_LIGHT_GRAY;
}

uint32_t tty_write(char *buf, uint32_t count)
{
    return tty_dev_write(&terminals[0], buf, count);
}

static void tty_cursor_going(dev_terminal_t *term, int n)
{
    for (int i = 0; i < n; i++) {
        if (++term->cc >= term->columns) {
            term->cr++;
            term->cc = 0;
        }
    }
}

static void tty_write_char(dev_terminal_t *term, char c)
{
    int offset = term->cc + term->cr * term->columns;
    tty_char_t *tc = (tty_char_t *)term->base + offset;
    tc->c = c;
    tc->fg = term->fg;
    tc->bg = term->bg;
    tty_cursor_going(term, 1);
}

uint32_t tty_dev_write(dev_terminal_t *term, char *buf, uint32_t count)
{
    char c;
    uint32_t nr = 0;
    while (nr < count)
    {
        c = *(buf + nr++);
        switch (c) {
            case ASCII_NUL: break;
            // case ASCII_BS: com_bs(); break;
            case ASCII_LF: com_lf(term); com_cr(term); /* ptr = (char*)pos; */ break;
            // case ASCII_FF: com_lf(); break;
            // case ASCII_CR: com_cr(); break;
            // case ASCII_DEL: com_del(); break;
            default: tty_write_char(term, c); break;
        }
    }
    return nr;
}

void tty_init()
{
    for (int i = 0; i < TTY_DEV_NR; i++) {
        dev_terminal_t *dev = &terminals[i];
        dev->base = (tty_char_t*) PM_VGA_BEGIN + i * (SCREEN_ROWS * SCREEN_COLUMNS);
        dev->columns = SCREEN_COLUMNS;
        dev->rows = SCREEN_ROWS;
        dev->cr = dev->cc = 0;
        dev->fg = COLOR_WHITE;
        dev->bg = COLOR_BLACK;
        tty_clear(dev);
    }
    mutex_init(&mutex);
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
    mutex_lock(&mutex);
    tty_write(buf, (uint32_t)i);
    mutex_unlock(&mutex);
    return i;
}

void usr_printf(char *msg, uint32_t length)
{
    mutex_lock(&mutex);
    tty_write(msg, length);
    mutex_unlock(&mutex);
}