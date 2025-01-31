#include <tty.h>
#include <device.h>
#include <logf.h>

static tty_t ttys[TTY_DEV_NR];

static void tty_fifo_init(tty_fifo_t* fifo, char *buf, int size)
{
    fifo->buf = buf;
    fifo->size = size;
    fifo->read = fifo->write = fifo->count = 0;
}

// 打开设备
int dev_tty_open(device_t *dev)
{
    int index = dev->minor;
    if (index < 0 || index >= TTY_DEV_NR) {
        tty_logf("open tty%d failed", index);
        return -1;
    }
    tty_t *tty = &ttys[index];
    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    tty->terminal_index = index;
    tty_init(index);
    return 0;
}

// 读取设备
int dev_tty_read(device_t *dev, int addr, char *buf, int size)
{
    return 0;
}

// 写入到设备
int dev_tty_write(device_t *dev, int addr, char *buf, int size)
{
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