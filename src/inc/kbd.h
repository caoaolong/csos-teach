#ifndef CSOS_KBD_H
#define CSOS_KBD_H

#include <kernel.h>

#define KBD_PORT_DATA   0x60
#define KBD_PORT_STAT   0x64

#define KBD_STATS_RR    (1 << 0)

#define KEY_E0              0xE0
#define KEY_LEFT_SHIFT      0x2A
#define KEY_RIGHT_SHIFT     0x36
#define KEY_CTRL            0x1D
#define KEY_ALT             0x38
#define KEY_CAPS            0x3A
#define KEY_ESC             0x01
#define KEY_ENTER           0x1C
#define KEY_F1              0x3B
#define KEY_F2              0x3C
#define KEY_F3              0x3D
#define KEY_F4              0x3E
#define KEY_F5              0x3F
#define KEY_F6              0x40
#define KEY_F7              0x41
#define KEY_F8              0x42
#define KEY_F9              0x43
#define KEY_F10             0x44
#define KEY_F11             0x57
#define KEY_F12             0x58
#define KEY_SPACE           0x39

typedef struct key_map_t {
    // 默认按键
    uint8_t normal;
    // 带有辅助按键的按键
    uint8_t func;
} key_map_t;

typedef struct key_state_t {
    int e0:1;
    int lp_shift:1;
    int rp_shift:1;
    int caps_lock:1;
    int lp_ctrl:1;
    int rp_ctrl:1;
    int lp_alt:1;
    int rp_alt:1;
} key_state_t;

void kbd_init();

#endif