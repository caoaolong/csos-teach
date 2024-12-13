#ifndef CSOS_TIMER_H
#define CSOS_TIMER_H

#include <interrupt.h>

#define OS_TICKS_MS         1

// 每秒钟时钟中断个数（时钟频率）
#define PIT_OSC_FREQ        1193182

#define PIT_CMD_MODE_PORT   0x43
#define PIT_CHL0_DATA_PORT  0x40
#define PIT_CHL2_DATA_PORT  0x42

#define PIT_CHANNLE0        (0 << 6)
#define PIT_CHANNLE2        (2 << 6)
#define PIT_LOAD_LOHI       (3 << 4)
#define PIT_MODE0           (3 << 1)

#define SPEAKER_REG         0x61
#define BEEP_HZ             440
#define BEEP_COUNTER        (PIT_OSC_FREQ / BEEP_HZ)

void timer_init();

void start_beep();

void stop_beep();

#endif