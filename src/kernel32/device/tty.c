#include <tty.h>
#include <device.h>

// 打开设备
int tty_open(device_t *dev)
{
    return 0;
}

// 读取设备
int tty_read(device_t *dev, int addr, char *buf, int size)
{
    return 0;
}

// 写入到设备
int tty_write(device_t *dev, int addr, char *buf, int size)
{
    return 0;
}

// 向设备发送命令
int tty_command(device_t *dev, int cmd, int arg0, int arg1)
{
    return 0;
}

// 关闭设备
void tty_close(device_t *dev)
{
    
}

device_handle_t device_tty = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .command = tty_command,
    .close = tty_close
};