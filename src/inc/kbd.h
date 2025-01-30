#ifndef CSOS_KBD_H
#define CSOS_KBD_H

#include <kernel.h>

#define KBD_PORT_DATA   0x60
#define KBD_PORT_STAT   0x64

#define KBD_STATS_RR    (1 << 0)

#define KEY_LEFT_SHIFT      0x2A
#define KEY_RIGHT_SHIFT     0x36

typedef struct key_map_t {
    // 默认按键
    uint8_t normal;
    // 带有辅助按键的按键
    uint8_t func;
} key_map_t;

typedef struct key_state_t {
    int lp_shift:1;
    int rp_shift:1;
} key_state_t;

void kbd_init();

#endif