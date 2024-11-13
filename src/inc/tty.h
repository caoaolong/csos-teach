#ifndef CSOS_TTY_H
#define CSOS_TTY_H

#include <types.h>

void tty_clear();

uint32_t tty_write(char *buf, uint32_t count);

void tty_init();

int tty_printf(const char *fmt, ...);

#endif