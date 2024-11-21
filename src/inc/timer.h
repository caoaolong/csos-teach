#ifndef CSOS_TIMER_H
#define CSOS_TIMER_H

#include <interrupt.h>

#define OS_TICKS_MS         10

// 每秒钟时钟中断个数（时钟频率）
#define PIT_OSC_FREQ        1193182

#define PIT_CMD_MODE_PORT   0x43
#define PIT_CHL0_DATA_PORT  0x40

#define PIT_CHANNLE0        (0 << 6)
#define PIT_LOAD_LOHI       (3 << 4)
#define PIT_MODE0           (3 << 1)

void timer_init();

#endif