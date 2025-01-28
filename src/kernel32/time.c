//
// Created by Administrator on 2022/10/26 0026.
//
#include <csos/time.h>
#include <csos/stdlib.h>
#include <rtc.h>
#include <tty.h>

#define MINUTE 60               // 每分钟的秒数
#define HOUR (60 * MINUTE)      // 每小时的秒数
#define DAY (24 * HOUR)         // 每天的秒数
#define YEAR (365 * DAY)        // 每年的秒数，以 365 天算

static int month_days[13] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// 每个月开始时的已经过去天数
static int month[13] = {
        0, // 这里占位，没有 0 月，从 1 月开始
        0,
        (31),
        (31 + 29),
        (31 + 29 + 31),
        (31 + 29 + 31 + 30),
        (31 + 29 + 31 + 30 + 31),
        (31 + 29 + 31 + 30 + 31 + 30),
        (31 + 29 + 31 + 30 + 31 + 30 + 31),
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)};

time_t startup_time;
int century;

time_t mktime(tm *time)
{
    time_t res;
    // year = time->tm_year + 100 - 70;
    int year = time->tm_year + 30;
    // 这些年经过的秒数时间
    res = YEAR * year;

    // 已经过去的闰年，每个加 1 天
    res += DAY * ((year + 1) / 4);

    // 已经过完的月份的时间
    res += month[time->tm_mon] * DAY;

    // 如果 2 月已经过了，并且当前不是闰年，那么减去一天
    if (time->tm_mon > 2 && ((year + 2) % 4))
        res -= DAY;

    // 这个月已经过去的天
    res += DAY * (time->tm_mday - 1);

    // 今天过去的小时
    res += HOUR * time->tm_hour;

    // 这个小时过去的分钟
    res += MINUTE * time->tm_min;

    // 这个分钟过去的秒
    res += time->tm_sec;

    return res;
}

void time_read_bcd(tm *time)
{
    do {
        time->tm_sec = cmos_read(CMOS_SECOND);
        time->tm_sec = cmos_read(CMOS_SECOND);
        time->tm_min = cmos_read(CMOS_MINUTE);
        time->tm_hour = cmos_read(CMOS_HOUR);
        time->tm_wday = cmos_read(CMOS_WEEKDAY);
        time->tm_mday = cmos_read(CMOS_DAY);
        time->tm_mon = cmos_read(CMOS_MONTH);
        time->tm_year = cmos_read(CMOS_YEAR);
        century = cmos_read(CMOS_CENTURY);
    } while (time->tm_sec != cmos_read(CMOS_SECOND));
}

void time_read(tm *time, int timezone)
{
    time_read_bcd(time);
    time->tm_sec = bcd_to_bin(time->tm_sec);
    time->tm_min = bcd_to_bin(time->tm_min);
    time->tm_hour = bcd_to_bin(time->tm_hour) + timezone;
    time->tm_wday = bcd_to_bin(time->tm_wday);
    time->tm_mday = bcd_to_bin(time->tm_mday);
    if (time->tm_hour >= 24) {
        time->tm_wday++;
        time->tm_mday++;
        time->tm_hour %= 24;
    }
    if (time->tm_wday >= 8) {
        time->tm_wday %= 8;
    }
    time->tm_mon = bcd_to_bin(time->tm_mon);
    int mdays = month_days[time->tm_mon];
    if (time->tm_mday > mdays) {
        time->tm_mday -= mdays;
        time->tm_mon ++;
    }
    time->tm_year = bcd_to_bin(time->tm_year);
    if (time->tm_mon > 12) {
        time->tm_year++;
    }
    time->tm_isdst = -1;
    century = bcd_to_bin(century);
    if (time->tm_year > 99) {
        century++;
        time->tm_year %= 100;
    }
}

void time_init(int timezone)
{
    tm time;
    time_read(&time, timezone);
    startup_time = mktime(&time);
    tty_color_set(COLOR_YELLOW);
    tty_printf("========================STARTUP TIME: %d%d-%02d-%02d %02d:%02d:%02d=======================",
         century,
         time.tm_year,
         time.tm_mon,
         time.tm_mday,
         time.tm_hour,
         time.tm_min,
         time.tm_sec);
    tty_color_reset();
}