#ifndef CSOS_TTY_H
#define CSOS_TTY_H

#include <types.h>
#include <csos/stdarg.h>
#include <csos/sem.h>

#define TTY_DEV_NR              8

#define PM_VGA_BEGIN            0xB8000
#define PM_VGA_END              0XBFFFF
#define SCREEN_ROWS             25
#define SCREEN_COLUMNS          80

#define COLOR_BLACK             0b0000
#define COLOR_BLUE              0b0001
#define COLOR_GREEN             0b0010
#define COLOR_CYAN              0b0011
#define COLOR_RED               0b0100
#define COLOR_MAGENTA           0b0101
#define COLOR_BROWN             0b0110
#define COLOR_LIGHT_GRAY        0b0111
#define COLOR_GRAY              0b1000
#define COLOR_LIGHT_BLUE        0b1001
#define COLOR_LIGHT_GREEN       0b1010
#define COLOR_LIGHT_CYAN        0b1011
#define COLOR_LIGHT_RED         0b1100
#define COLOR_LIGHT_MAGENTA     0b1101
#define COLOR_YELLOW            0b1110
#define COLOR_WHITE             0b1111

typedef union tty_char_t {
    struct {
        char c;
        char fg:4;
        char bg:3;
        char blink:1;
    };
    uint16_t v;
} tty_char_t;


typedef struct dev_terminal_t {
    // 屏幕的内存位置
    tty_char_t *base;
    // 屏幕当前显示模式的行列数
    int rows, columns;
    // 原先光标所在行列数
    int or, oc;
    // 光标所在行列数
    int cr, cc;
    // 颜色
    uint8_t fg, bg;
    // 当前字符属性
    uint8_t cfg:4;
    uint8_t cbg:3;
    uint8_t cb:1;
} dev_terminal_t;

typedef struct tty_fifo_t {
    char *buf;
    int size;
    int read, write;
    int count;
} tty_fifo_t;

#define TTY_OBUF_SIZE   512
#define TTY_IBUF_SIZE   512

typedef struct tty_t {
    char obuf[TTY_OBUF_SIZE];
    tty_fifo_t ofifo;
    char ibuf[TTY_IBUF_SIZE];
    tty_fifo_t ififo;
    sem_t osem;
    int terminal_index;
} tty_t;

// fifo functions

int tty_fifo_get(tty_fifo_t* fifo, char *c);

int tty_fifo_put(tty_fifo_t* fifo, char c);

// terminal functions

void tty_clear(dev_terminal_t *term);

uint32_t tty_write(tty_t *tty);

void tty_init();

int tty_printf(const char *fmt, ...);

void tty_init(int index);

#endif