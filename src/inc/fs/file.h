#ifndef CSOS_FILE_H
#define CSOS_FILE_H

#include <types.h>

#define stdin       0
#define stdout      1
#define stderr      2

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

typedef enum file_type_t {
    FT_NUKNOWN, FT_TTY, FT_FILE, FT_DIR
} file_type_t;

typedef enum file_mode_t {
    FM_READ  = 0b00000001, 
    FM_WRITE = 0b00000010
} file_mode_t;

typedef struct FILE {
    // fd
    int fd;
    // 文件名称
    char name[FILE_NAME_SIZE];
    // 文件类型
    file_type_t type;
    // 文件大小
    uint32_t size;
    // 磁盘设备ID
    uint8_t devid;
    // 当前文件目录的所在扇区和偏移量
    uint32_t dcluster;
    uint32_t doffset;
    // 当前文件指针的偏移量
    uint32_t offset;
    // 文件数据开始块（簇）
    uint32_t sblk;
    // 当前文件指针所在块（簇）
    uint32_t cblk;
    // 文件打开模式
    uint8_t mode;
    // 引用数量
    int ref;
} FILE;

void printf(const char *fmt, ...);
void fprintf(int fd, const char *fmt, ...);

int fopen(const char *filepath, const char *mode);
int fgets(int fd, char *buf, uint32_t size);
int fputs(int fd, char *buf, uint32_t size);
int fclose(int fd);

int lseek(int fd, int offset, int dir);
int remove(const char *path);

char getc();
void putc();
int dup(int fd);

#endif