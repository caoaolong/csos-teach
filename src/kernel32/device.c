#include <device.h>

#define DEVICE_TABLE_SIZE   1024

extern device_handle_t device_tty;

static device_handle_t *dht[] = {
    &device_tty,
};

static device_t dt[DEVICE_TABLE_SIZE];

int device_open(int major, int minor, void *data)
{
    return 0;
}

int device_read(int device_id, int addr, char *buf, int size)
{
    return 0;
}

int device_write(int device_id, int addr, char *buf, int size)
{
    return 0;
}

int device_command(int device_id, int cmd, int arg0, int arg1)
{
    return 0;
}

void device_close(int device_id)
{

}