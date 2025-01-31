#ifndef CSOS_DEVICE_H
#define CSOS_DEVICE_H

#define DEVICE_NAME_SIZE    32

enum {
    DEV_UNKNOWN = 0,
    DEV_TTY
};

struct device_handle_t;

typedef struct device_t {
    struct device_handle_t *handle;
    int mode;
    int minor;
    void *data;
    int opc;
} device_t;

typedef struct device_handle_t {
    char name[DEVICE_NAME_SIZE];
    int major;
    // 打开设备
    int (*open)(device_t *dev);
    // 读取设备
    int (*read)(device_t *dev, int addr, char *buf, int size);
    // 写入到设备
    int (*write)(device_t *dev, int addr, char *buf, int size);
    // 向设备发送命令
    int (*command)(device_t *dev, int cmd, int arg0, int arg1);
    // 关闭设备
    void (*close)(device_t *dev);
} device_handle_t;

#endif