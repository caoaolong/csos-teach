#ifndef CSOS_TIME_H
#define CSOS_TIME_H

#include <kernel.h>

typedef struct tm
{
    int tm_sec;     //秒
    int tm_min;     // 分
    int tm_hour;    // 时
    int tm_mday;    // 天
    int tm_mon;     // 月
    int tm_year;    // 从1900年开始的年数
    int tm_wday;    // 1星期中的第几天
    int tm_isdst;   // 夏时令标志
} tm;

typedef uint32_t time_t;

void time_read_bcd(tm *time);

void time_read(tm *time, int timezone);

time_t mktime(tm *time);

void time_init(int timezone);

#endif
