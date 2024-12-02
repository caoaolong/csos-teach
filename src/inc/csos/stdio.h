#ifndef CSOS_STDIO_H
#define CSOS_STDIO_H

#include <csos/stdarg.h>

int vsprintf(char *buf, const char* fmt, va_list args);

#endif