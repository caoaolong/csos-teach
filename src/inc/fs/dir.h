#ifndef CSOS_DIR_H
#define CSOS_DIR_H

#include <types.h>

struct dirent {
    int d_ino;                      // 文件的 inode 编号
    uint32_t d_reclen;              // 当前 dirent 的长度
    uint8_t  d_type;                // 文件类型（如普通文件、目录等）
    char d_name[FILE_NAME_SIZE];    // 文件名（以 null 结尾的字符串）
};

typedef struct DIR {
    int index;
    struct dirent dirent;
} DIR;

DIR *opendir(const char *path);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

#endif