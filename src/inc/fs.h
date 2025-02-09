#ifndef CSOS_FS_H
#define CSOS_FS_H

#include <kernel.h>
#include <list.h>
#include <csos/stdarg.h>
#include <csos/mutex.h>

// 文件系统挂载点
#define FS_TABLE_SIZE       10
#define FILE_NAME_SIZE      32

typedef enum file_type_t {
    FT_NUKNOWN, FILE_TTY
} file_type_t;

typedef enum fs_type_t {
    FS_DEV
} fs_type_t;

#define MOUNT_POINT_SIZE    32

struct fs_op_t;
typedef struct fs_t {
    char mp[MOUNT_POINT_SIZE];
    struct fs_op_t *op;
    fs_type_t type;
    void *data;
    int devid;
    list_node_t node;
    mutex_t *mutex;
} fs_t;

typedef struct stat_t {
} stat_t;

typedef struct file_t {
    char name[FILE_NAME_SIZE];
    file_type_t type;
    uint32_t size;
    uint32_t ref;
    uint8_t devid;
    uint32_t position;
    uint8_t mode;
} file_t;

typedef struct fs_op_t {
    int (*mount)(fs_t *fs, int major, int minor);
    void (*unmount)(fs_t *fs);
    int (*open)(fs_t *fs, const char *path, file_t *file);
    int (*read)(char *buf, int size, file_t *file);
    int (*write)(char *buf, int size, file_t *file);
    void (*close)(file_t *file);
    int (*seek)(file_t *file, uint32_t offset, int dir);
    int (*stat)(file_t *file, stat_t *st);
} fs_op_t;

void fs_init();

int fs_open(const char *name, int flags, ...);
int fs_read(int file, char *buf, int len);
int fs_write(int file, char *buf, int len);
int fs_lseek(int file, int pos, int dir);
int fs_close(int file);
int fs_isatty(int file);
int fs_fstat(int file);
char *fs_sbrk(int size);

#endif