#ifndef CSOS_LOGF_H
#define CSOS_LOGF_H

#define COM1_PORT   0x3F8

void tty_logf_init();

void tty_logf(const char * fmt, ...);

#endif