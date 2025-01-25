#ifndef CSOS_TTY_H
#define CSOS_TTY_H

#include <types.h>
#include <csos/stdarg.h>

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


void tty_clear();

void tty_color_set(uint8_t color);

void tty_color_reset();

uint32_t tty_write(char *buf, uint32_t count);

void tty_init();

int tty_printf(const char *fmt, ...);

void usr_printf(char *msg, uint32_t length);

#endif