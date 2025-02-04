#include <device.h>
#include <interrupt.h>
#include <csos/string.h>

#define DEVICE_TABLE_SIZE   1024

extern device_handle_t device_tty;
extern device_handle_t device_disk;

static device_handle_t *dht[] = {
    &device_tty, &device_disk
};

static device_t dt[DEVICE_TABLE_SIZE];

static BOOL verify_device_id(int device_id)
{
    if (device_id < 0 || device_id >= DEVICE_TABLE_SIZE) {
        return FLASE;
    }
    if (dt[device_id].handle == NULL) {
        return FLASE;
    }
    return TRUE;
}

int device_open(int major, int minor, void *data)
{
    protect_state_t state = protect_enter();
    device_t *dev = NULL;
    int idev;
    for (idev = 0; idev < sizeof(dt) / sizeof(device_t); idev++) {
        if (dt[idev].opc == 0) {
            dev = &dt[idev];
            break;
        } else if (dt[idev].handle->major == major && dt[idev].minor == minor) {
            dt[idev].opc++;
            protect_exit(state);
            return idev;
        }
    }
    device_handle_t *handle = NULL;
    for (int i = 0; i < sizeof(dht) / sizeof(void*); i++) {
        if (dht[i]->major == major) {
            handle = dht[i];
            break;
        }
    }

    if (handle && dev) {
        dev->minor = minor;
        dev->data = data;
        dev->handle = handle;
        int err = handle->open(dev);
        if (err == 0) {
            dev->opc = 1;
            protect_exit(state);
            return idev;
        }
    }
    protect_exit(state);
    return -1;
}

int device_read(int device_id, int addr, char *buf, int size)
{
    if(!verify_device_id(device_id)) 
        return -1;
    device_t *dev = &dt[device_id];
    return dev->handle->read(dev, addr, buf, size);
}

int device_write(int device_id, int addr, char *buf, int size)
{
    if(!verify_device_id(device_id)) 
        return -1;
    device_t *dev = &dt[device_id];
    return dev->handle->write(dev, addr, buf, size);
}

int device_command(int device_id, int cmd, int arg0, int arg1)
{
    if(!verify_device_id(device_id)) 
        return -1;
    device_t *dev = &dt[device_id];
    return dev->handle->command(dev, cmd, arg0, arg1);
}

void device_close(int device_id)
{
    if(!verify_device_id(device_id)) 
        return;
    device_t *dev = &dt[device_id];
    protect_state_t state = protect_enter();
    if (--dev->opc == 0) {
        dev->handle->close(dev);
        kernel_memset(dev, 0, sizeof(device_t));
    }
    protect_exit(state);
}