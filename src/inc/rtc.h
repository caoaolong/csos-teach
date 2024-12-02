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

#define CMOS_SECOND 0x01
#define CMOS_MINUTE 0x03
#define CMOS_HOUR   0x05

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