#ifndef CSOS_FS_H
#define CSOS_FS_H

#include <kernel.h>
#include <list.h>
#include <fs/fat.h>
#include <fs/dir.h>
#include <fs/file.h>
#include <csos/stdarg.h>
#include <csos/mutex.h>

// 文件系统挂载点
#define FS_TABLE_SIZE       10
#define FILE_NAME_SIZE      32

typedef enum fs_type_t {
    FS_DEV, FS_FAT
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
    // 文件系统数据
    union {
        fs_fat_t fat_data;
    };
} fs_t;

typedef struct stat_t {
} stat_t;

typedef struct fs_op_t {
    int (*mount)(fs_t *fs, int major, int minor);
    void (*unmount)(fs_t *fs);
    int (*fopen)(fs_t *fs, FILE *file, const char *path, const char *mode);
    int (*fread)(fs_t *fs, FILE *file, char *buf, int size);
    int (*fwrite)(fs_t *fs, FILE *file, char *buf, int size);
    void (*fclose)(fs_t *fs, FILE *file);
    int (*lseek)(fs_t *fs, FILE *file, uint32_t offset, int dir);
    int (*fstat)(fs_t *fs, FILE *file, stat_t *st);
    int (*opendir)(fs_t *fs, const char *path, DIR *dir);
    int (*readdir)(fs_t *fs, DIR *dir, struct dirent *dirent);
    int (*closedir)(fs_t *fs, DIR *dir);
} fs_op_t;

void fs_init();

int fs_fopen(FILE *file, const char *filepath, const char *mode);
int fs_fread(FILE *file, char *buf, int size);
int fs_fwrite(FILE *file, char *buf, int size);
int fs_lseek(FILE *file, int pos, int dir);
int fs_fclose(FILE *file);
int fs_isatty(int file);
int fs_fstat(int file);
char *fs_sbrk(int size);

int fs_opendir(const char *path, DIR *dir);
int fs_readdir(DIR *dir, struct dirent *dirent);
int fs_closedir(DIR *dir);

#endif