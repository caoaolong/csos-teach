#ifndef CSOS_LOGF_H
#define CSOS_LOGF_H

#define COM1_PORT   0x3F8

void logf_init();

void logf(const char * fmt, ...);

#endif