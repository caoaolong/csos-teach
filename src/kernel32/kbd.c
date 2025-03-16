#include <kbd.h>
#include <interrupt.h>
#include <pic.h>
#include <kernel.h>
#include <logf.h>
#include <device.h>
#include <tty.h>
#include <csos/string.h>

static const key_map_t kmt[] = {
    [0x29] = {'`', '~'},
    [0x2] = {'1', '!'},
    [0x3] = {'2', '@'},
    [0x4] = {'3', '#'},
    [0x5] = {'4', '$'},
    [0x6] = {'5', '%'},
    [0x7] = {'6', '^'},
    [0x8] = {'7', '&'},
    [0x9] = {'8', '*'},
    [0xA] = {'9', '('},
    [0xB] = {'0', ')'},
    [0xC] = {'-', '_'},
    [0xD] = {'=', '+'},
    [0xE] = {'\b', '\b'},
    [0xF] = {'\t', '\t'},
    [0x1E] = {'a', 'A'},
    [0x30] = {'b', 'B'},
    [0x2E] = {'c', 'C'},
    [0x20] = {'d', 'D'},
    [0x12] = {'e', 'E'},
    [0x21] = {'f', 'F'},
    [0x22] = {'g', 'G'},
    [0x23] = {'h', 'H'},
    [0x17] = {'i', 'I'},
    [0x24] = {'j', 'J'},
    [0x25] = {'k', 'K'},
    [0x26] = {'l', 'L'},
    [0x32] = {'m', 'M'},
    [0x31] = {'n', 'N'},
    [0x18] = {'o', 'O'},
    [0x19] = {'p', 'P'},
    [0x10] = {'q', 'Q'},
    [0x13] = {'r', 'R'},
    [0x1F] = {'s', 'S'},
    [0x14] = {'t', 'T'},
    [0x16] = {'u', 'U'},
    [0x2F] = {'v', 'V'},
    [0x11] = {'w', 'W'},
    [0x2D] = {'x', 'X'},
    [0x15] = {'y', 'Y'},
    [0x2C] = {'z', 'Z'},
    [0x52] = {'0', '0'},
    [0x53] = {'.', '.'},
    [0x4F] = {'1', '1'},
    [0x50] = {'2', '2'},
    [0x51] = {'3', '3'},
    [0x4B] = {'4', '4'},
    [0x4C] = {'5', '5'},
    [0x4D] = {'6', '6'},
    [0x47] = {'7', '7'},
    [0x48] = {'8', '8'},
    [0x49] = {'9', '9'},
    [0x4A] = {'-', '-'},
    [0x4E] = {'+', '+'},
    [0x45] = {'/', '/'},
    [0x46] = {'/', '/'},
    [0x37] = {'*', '*'},
    [0x33] = {',', '<'},
    [0x34] = {'.', '>'},
    [0x35] = {'/', '?'},
    [0x27] = {';', ':'},
    [0x28] = {'\'', '"'},
    [0x1A] = {'[', '{'},
    [0x1B] = {']', '}'},
    [0x2B] = {'\\', '|'}
};

static key_state_t ks;
static int tty_now = 0;

#define MAKE_CODE(rc)       !((rc) & 0x80)
#define KEY_CODE(rc)        ((rc) & 0x7F)

extern tty_t ttys[TTY_DEV_NR];

static void tty_put(char ch)
{
    tty_t *tty = &ttys[tty_now];
    if (sem_count(&tty->isem) >= TTY_IBUF_SIZE) {
        return;
    }
    tty_fifo_put(&tty->ififo, ch);
    sem_notify(&tty->isem);
}

static void handle_fx(char key)
{
    int index = key - KEY_F1;
    if (index != tty_now) {
        tty_select(index);
        tty_now = index;
    }
    tty_t *tty = &ttys[tty_now];
    sem_notify(&tty->isem);
}

static void handle_normal_key(uint8_t rc)
{
    // 处理双字节按键
    if (rc == KEY_E0) 
        ks.e0 = 1;
    uint8_t is_make = MAKE_CODE(rc);
    uint8_t key = KEY_CODE(rc);
    switch (key) {
        case KEY_F1:
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
            handle_fx(key);
            break;
        case KEY_F9:
        case KEY_F10:
        case KEY_F11:
        case KEY_F12:
            logf("F-Keys: %d", key);
            break;
        case KEY_ESC:
            if (is_make) logf("ESC");
            break;
        case KEY_ENTER:
            {char enter = '\n';
            if (is_make) tty_put(enter);
            break;}
        case KEY_SPACE:
            {char space = ' ';
            if (is_make) tty_put(space);
            break;}
        case KEY_BACKSPACE:
            {char bs = '\b';
            if (is_make) tty_put(bs);
            break;}
        case KEY_TAB:
            {char tb = '\t';
            if (is_make) tty_put(tb);
            break;}
        case KEY_CAPS:
            if (is_make) ks.caps_lock = ~ks.caps_lock;
            break;
        case KEY_CTRL:
            if (ks.e0) {
                if (is_make) logf("Right Ctrl");
                ks.rp_ctrl = is_make;
                ks.e0 = 0;
            } else {
                if (is_make) logf("Left Ctrl");
                ks.lp_ctrl = is_make;
            }
            break;
        case KEY_ALT:
            if (ks.e0) {
                if (is_make) logf("Right Alt");
                ks.rp_alt = is_make;
                ks.e0 = 0;
            } else {
                if (is_make) logf("Left Alt");
                ks.lp_alt = is_make;
            }
            break;
        case KEY_LEFT_SHIFT:
            ks.lp_shift = is_make;
            break;
        case KEY_RIGHT_SHIFT:
            ks.rp_shift = is_make;
            break;
        default:
            if (is_make) {
                // 处理CapsLock和Shift
                if (ks.lp_shift || ks.rp_shift) {
                    key = ks.caps_lock ? kmt[key].normal : kmt[key].func;
                } else {
                    key = ks.caps_lock ? kmt[key].func : kmt[key].normal;
                }
                tty_put(key);
            }
            break;
    }
}

static void handle_func_key(uint8_t rc)
{

}

void handler_kbd(interrupt_frame_t* frame)
{
    uint8_t status = inb(KBD_PORT_STAT);
    if (!(status & KBD_STATS_RR)) {
        send_eoi(IRQ1_KBD);
        return;
    }
    uint8_t rc = inb(KBD_PORT_DATA);
    handle_normal_key(rc);
    send_eoi(IRQ1_KBD);
}

void kbd_init()
{
    kernel_memset(&ks, 0, sizeof(key_state_t));
    install_interrupt_handler(IRQ1_KBD, (uint32_t)interrupt_handler_kbd);
    irq_enable(IRQ1_KBD);
}