#ifndef CSOS_FILE_H
#define CSOS_FILE_H

#include <types.h>

typedef enum file_type_t {
    FT_NUKNOWN, FT_TTY, FT_FILE, FT_DIR
} file_type_t;

typedef struct FILE {
    // 文件名称
    char name[FILE_NAME_SIZE];
    // 文件类型
    file_type_t type;
    // 文件大小
    uint32_t size;
    // 磁盘设备ID
    uint8_t devid;
    // 当前文件指针的偏移量
    uint32_t offset;
    // 文件数据开始块（簇）
    fat_cluster_t sblk;
    // 当前文件指针所在块（簇）
    fat_cluster_t cblk;
    // 文件打开模式
    uint8_t mode;
} FILE;

FILE *fopen(const char *filepath, const char *mode);
int fgets(FILE *file, char *buf, uint32_t size);
int fclose(FILE *file);

#endif