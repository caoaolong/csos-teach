#ifndef CSOS_RTC_H
#define CSOS_RTC_H

#include <kernel.h>

#define CMOS_ADDR   0x70
#define CMOS_DATA   0x71
#define CMOS_NMI    0x80

#define CMOS_A      0x0A
#define CMOS_B      0x0B
#define CMOS_C      0x0C
#define CMOS_D      0x0D

// 下面是 CMOS 信息的寄存器索引
#define CMOS_SECOND 0x00        // (0 ~ 59)
#define CMOS_MINUTE 0x02        // (0 ~ 59)
#define CMOS_HOUR 0x04          // (0 ~ 23)
#define CMOS_WEEKDAY 0x06       // (1 ~ 7) 星期天 = 1，星期六 = 7
#define CMOS_DAY 0x07           // (1 ~ 31)
#define CMOS_MONTH 0x08         // (1 ~ 12)
#define CMOS_YEAR 0x09          // (0 ~ 99)
#define CMOS_CENTURY 0x32       // 可能不存在

#define CMOS_B_24HOUR   (1 << 1)
#define CMOS_B_AIE      (1 << 5)
#define CMOS_B_PIE      (1 << 6)
#define CMOS_C_PF       (1 << 6)
#define CMOS_C_AF       (1 << 5)

void rtc_init();

void create_alarm(uint32_t value);

uint8_t cmos_read(uint8_t addr);
void cmos_write(uint8_t addr, uint8_t value);

#endif