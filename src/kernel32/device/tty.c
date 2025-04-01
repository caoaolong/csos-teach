#include <tty.h>
#include <kbd.h>
#include <device.h>
#include <logf.h>
#include <interrupt.h>
#include <csos/mutex.h>
#include <csos/string.h>
#include <csos/stdio.h>

#define CRT_ADDR_REG        0x3D4
#define CRT_DATA_REG        0x3D5

#define CRT_START_ADDR_H    0xC
#define CRT_START_ADDR_L    0xD
#define CRT_CURSOR_H        0xE
#define CRT_CURSOR_L        0xF

#define ASCII_NUL   0x00
#define ASCII_ENQ   0x05
#define ASCII_BEL   0x07
#define ASCII_BS    0x08
#define ASCII_HT    0x09
#define ASCII_LF    0x0A
#define ASCII_VT    0x0B
#define ASCII_FF    0x0C
#define ASCII_CR    0x0D
#define ASCII_DEL   0x0F
#define ASCII_ESC   '\033'

mutex_t mutex;
tty_t ttys[TTY_DEV_NR];
static dev_terminal_t terminals[TTY_DEV_NR];
static int tty_now = 0;

static void set_cursor(dev_terminal_t *term)
{
    int cursor = (term - terminals) * term->columns * term->rows;
    cursor += term->cr * term->columns + term->cc;
    protect_state_t ps = protect_enter();
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, ((uint8_t)(cursor >> 8)) & 0xFF);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, ((uint8_t)cursor) & 0xFF);
    protect_exit(ps);
}

static void tty_fifo_init(tty_fifo_t* fifo, char *buf, int size)
{
    fifo->buf = buf;
    fifo->size = size;
    fifo->read = fifo->write = fifo->count = 0;
}

int tty_fifo_put(tty_fifo_t* fifo, char c)
{
    protect_state_t ps = protect_enter();
    if (fifo->count >= fifo->size) return -1;
    fifo->buf[fifo->write++] = c;
    if (fifo->write >= fifo->size) fifo->write = 0;
    fifo->count++;
    protect_exit(ps);
    return 0;
}

int tty_fifo_get(tty_fifo_t* fifo, char *c)
{
    protect_state_t ps = protect_enter();
    if (fifo->count <= 0) return -1;
    *c = fifo->buf[fifo->read++];
    if (fifo->read >= fifo->size) fifo->read = 0;
    fifo->count--;
    protect_exit(ps);
    return 0;
}

static tty_t *get_tty(device_t *device)
{
    int tty = device->minor;
    if (tty < 0 || tty >= TTY_DEV_NR || !device->opc) {
        logf("tty%d is not opened", tty);
        return NULL;
    }
    return &ttys[tty];
}

void sys_clear()
{
    tty_t *tty = &ttys[tty_now];
    dev_terminal_t *term = &terminals[tty->terminal_index];
    tty_clear(term);
    term->cr = term->cc = 0;
    set_cursor(term);
}

void tty_clear(dev_terminal_t *term)
{
    int size = term->columns * term->rows;
    for (int i = 0; i < size; i++) {
        tty_char_t *tc = term->base + i;
        tc->c = ' ';
        tc->bg = term->bg;
        tc->fg = term->fg;
    }
}

static void com_cr(dev_terminal_t *term)
{
    term->cc = 0;
}

static void scroll_up(dev_terminal_t *term, int lines)
{
    // 向上滚动n行
    int total = term->columns * term->rows;
    int size = term->columns * lines;
    tty_char_t *dst = term->base;
    tty_char_t *src = term->base + size;
    kernel_memcpy((char*)dst, (char*)src, (total - size) * 2);
    // 清空最后n行
    tty_char_t *ptr = term->base + (total - size);
    for (int i = 0; i < size; i++) {
        (ptr + i)->c = ' ';
    }
}

static void com_lf(dev_terminal_t *term)
{
    if (term->cr + 1 < term->rows) {
        term->cr++;
        return;
    }
    scroll_up(term, 1);
}

static void tty_cursor_backword(dev_terminal_t *term, int n)
{
    if (term->cc == 0 && term->cr == 0) return;

    for (int i = 0; i < n; i++) {
        if (--term->cc < 0) {
            term->cr--;
            term->cc = term->columns - 1;
        }
    }
}

static void tty_cursor_forward(dev_terminal_t *term, int n)
{
    for (int i = 0; i < n; i++) {
        if (++term->cc >= term->columns) {
            term->cr++;
            term->cc = 0;
        }
    }
}

static void com_left(dev_terminal_t *term, tty_t *tty)
{
    if (!tty->cursor.can_left)
        return;
    tty_cursor_backword(term, 1);
}

static void com_right(dev_terminal_t *term, tty_t *tty)
{
    if (!tty->cursor.can_right)
        return;
    tty_cursor_forward(term, 1);
}

static void tty_write_char(dev_terminal_t *term, tty_t *tty, char c)
{
    if (c == KEY_LEFT && !tty->cursor.can_echo) {
        com_left(term, tty);
        return;
    }

    if (c == KEY_RIGHT && !tty->cursor.can_echo) {
        com_right(term, tty);
        return;
    }

    if ((c == KEY_UP || c == KEY_DOWN) && !tty->cursor.can_echo) {
        return;
    }

    int offset = term->cc + term->cr * term->columns;
    if (offset >= term->columns * term->rows) {
        scroll_up(term, 1);
    }
    tty_char_t *tc = (tty_char_t *)term->base + offset;
    if (tty->cursor.current == tty->cursor.total) {
        tc->c = c;
        tc->fg = term->cfg;
        tc->bg = term->cbg;
        tc->blink = term->cb;
    } else {
        // 将指针移到尾部
        tty_char_t *ptc = tc + (tty->cursor.total - tty->cursor.current);
        while (ptc-- != tc) {
            (ptc + 1)->c = ptc->c;
            (ptc + 1)->fg = ptc->fg;
            (ptc + 1)->bg = ptc->bg;
            (ptc + 1)->blink = ptc->blink;
        }
    }
    tty_cursor_forward(term, 1);
}

static void com_esc(dev_terminal_t *term, tty_t *tty)
{
    tty_fifo_t *ofifo = &tty->ofifo;
    char c;
    int err = tty_fifo_get(ofifo, &c);
    if (err < 0) return;
    sem_notify(&tty->osem);
    if (c != '[') return;
    err = tty_fifo_get(ofifo, &c);
    if (err < 0) return;
    sem_notify(&tty->osem);
    uint8_t args[3];
    uint8_t idx = 0;
    while (TRUE) {
        if (c == 'm' || c == 'f' || c == 'H') break;
        if (c == ';') {
            err = tty_fifo_get(ofifo, &c);
            if (err < 0) break;
            sem_notify(&tty->osem);
            continue;
        }
        // 获取参数
        uint8_t arg = 0;
        while (c <= '9' && c >= '0') {
            arg = arg * 10 + (c - '0');
            err = tty_fifo_get(ofifo, &c);
            if (err < 0) break;
            sem_notify(&tty->osem);
        }
        args[idx++] = arg;
    }
    if (c == 'm') // SGR
    {
        for(int i = 0; i < idx; i++) {
            uint8_t arg = args[i];
            if (arg >= 30 && arg <= 37) // 设置前景色
            {
                term->cfg = arg - 30;
            }
            else if (arg >= 40 && arg <= 47) // 设置背景色
            {
                term->cbg = arg - 40;
            }
            else if (arg == 5 || arg == 6) // 开启闪烁
            {
                term->cb = 1;
            }
            else if (arg == 25) // 关闭闪烁
            {
                term->cb = 0;
            }
            else if (arg == 0) // 重置属性
            {
                term->cfg = term->fg;
                term->cbg = term->bg;
                term->cb = 0;
            }
        }
    } 
    else if (c == 'f' || c == 'H') // CUP / HVP
    {
        if (idx == 2) {
            term->or = term->cr;
            term->oc = term->cc;
            term->cr = args[0];
            term->cc = args[1];
        } else if (idx == 1 && args[0] == 0) {
            term->cr = term->or;
            term->cc = term->oc;
        }
    }
}

static void com_bs(dev_terminal_t *term, tty_t *tty)
{
    if (!tty->cursor.can_backspace)
        return;
    
    tty_char_t *tc = term->base + term->cc + term->cr * term->columns - 1;
    // 处理换行
    int size = 1;
    int backcount = tty->cursor.total - tty->cursor.current + 1;
    tty_char_t *ptc = tc;
    while (backcount--) {
        ptc->c = (ptc + 1)->c;
        ptc++;
    }
    if (term->cc == 0) {
        size = 0;
        while (tc->c == ' ') {
            size++;
            tc--;
        }
    }
    tty_cursor_backword(term, size);
}

static void com_ht(dev_terminal_t *term)
{
    int size = 8 - (term->cc % 8);
    tty_char_t *tc = term->base + term->cc + term->cr * term->columns;
    
    if (term->cc + size >= term->columns)
        size = term->columns - term->cc - 1;
    
    for (int i = 0; i < size; i++)
        (tc + i)->c = ' ';

    tty_cursor_forward(term, size);
}

uint32_t tty_write(tty_t *tty)
{
    dev_terminal_t *term = &terminals[tty->terminal_index];
    int len = 0;
    do {
        char c;
        int err = tty_fifo_get(&tty->ofifo, &c);
        if (err < 0) break;
        sem_notify(&tty->osem);
        switch (c) {
            case ASCII_ESC: com_esc(term, tty); break;
            case ASCII_BS: com_bs(term, tty); break;
            case ASCII_HT: com_ht(term); break;
            case ASCII_NUL: break;
            case ASCII_CR: com_cr(term); len++; break;
            case ASCII_LF: com_lf(term); com_cr(term); len++; break;
            default: tty_write_char(term, tty, c); len++; break;
        }
    } while(TRUE);
    set_cursor(term);
    return len;
}

void tty_select(int index)
{
    dev_terminal_t *term = &terminals[index];
    if (term->base == 0) {
        tty_init(index);
    }
    uint16_t poff = index * term->columns * term->rows;
    outb(CRT_ADDR_REG, 0xC);
    outb(CRT_DATA_REG, (uint8_t)((poff >> 8) & 0xFF));
    outb(CRT_ADDR_REG, 0xD);
    outb(CRT_DATA_REG, (uint8_t)(poff & 0xFF));
    set_cursor(term);
    tty_now = index;
}

// 打开设备
int dev_tty_open(device_t *dev)
{
    int index = dev->minor;
    if (index < 0 || index >= TTY_DEV_NR) {
        logf("open tty%d failed", index);
        return -1;
    }
    tty_t *tty = &ttys[index];

    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    sem_init(&tty->isem, 0);
    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    sem_init(&tty->osem, TTY_OBUF_SIZE);
    
    // 设置回显
    tty->iflags = TTY_IECHO;
    tty->cursor.can_backspace = tty->cursor.can_left = tty->cursor.can_right = 0;
    tty->cursor.current = tty->cursor.total = 0;

    tty->terminal_index = index;
    tty_init(index);
    return 0;
}

// 读取设备
int dev_tty_read(device_t *dev, int addr, char *buf, int size)
{
    if (size <= 0) return 0;
    tty_t *tty = get_tty(dev) + addr;
    char *pbuf = buf;
    int len = 0;
    while (size) {
        sem_wait(&tty->isem);
        char ch;
        int err = tty_fifo_get(&tty->ififo, &ch);
        if (err < 0) break;
        *pbuf++ = ch;
        if (tty->iflags & TTY_IECHO) {
            dev_tty_write(dev, tty_now, &ch, 1);
        }
        len++;
        size--;
    }
    return 0;
}

// 写入到设备
int dev_tty_write(device_t *dev, int addr, char *buf, int size)
{
    if (size <= 0) return 0;
    tty_t *tty = get_tty(dev) + addr;
    if (!tty) return -1;
    int len = 0;
    while (size) {
        char c = *buf++;
        sem_wait(&tty->osem);
        int err = tty_fifo_put(&tty->ofifo, c);
        if (err < 0) break;
        len++;
        size--;
    }
    tty_write(tty);
    return 0;
}

// 向设备发送命令
int dev_tty_command(device_t *dev, int cmd, int arg0, int arg1)
{
    return 0;
}

// 关闭设备
void dev_tty_close(device_t *dev)
{
    tty_t *tty = get_tty(dev);
    dev_terminal_t *term = &terminals[tty->terminal_index];
    term->base = 0;
}

device_handle_t device_tty = {
    .name = "tty",
    .major = DEV_TTY,
    .open = dev_tty_open,
    .read = dev_tty_read,
    .write = dev_tty_write,
    .command = dev_tty_command,
    .close = dev_tty_close
};

void tty_init(int index)
{
    dev_terminal_t *dev = &terminals[index];
    dev->base = (tty_char_t*) PM_VGA_BEGIN + index * (SCREEN_ROWS * SCREEN_COLUMNS);
    dev->columns = SCREEN_COLUMNS;
    dev->rows = SCREEN_ROWS;
    dev->or = dev->oc = dev->cr = dev->cc = dev->cb = 0;
    dev->fg = dev->cfg = COLOR_WHITE;
    dev->bg = dev->cbg = COLOR_BLACK;
    tty_clear(dev);
    mutex_init(&mutex);
}

int tty_tcgetattr(int fd, term_t *term)
{
    tty_t *tty = &ttys[tty_now];
    dev_terminal_t *terminal = &terminals[tty->terminal_index];
    if (fd == stdin) {
        term->cc = terminal->cc;
        term->cr = terminal->cr;
    }
    return 0;
}

int tty_tcsetattr(int fd, term_t *term)
{
    tty_t *tty = &ttys[tty_now];
    dev_terminal_t *terminal = &terminals[tty->terminal_index];
    if (fd == stdin) {
        terminal->cc = term->cc;
        terminal->cr = term->cr;
        set_cursor(terminal);
    }
    int count = terminal->columns * terminal->rows - (terminal->columns * terminal->cr + terminal->cc);
    tty_char_t *start = terminal->base + (terminal->columns * terminal->cr + terminal->cc);
    for (int i = 0; i < count; i++) {
        start->c = ' ';
        start->fg = terminal->fg;
        start->bg = terminal->bg;
        start->blink = terminal->cb;
        start++;
    }
    return 0;
}