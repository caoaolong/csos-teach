#ifndef CSOS_FILE_H
#define CSOS_FILE_H

#include <types.h>

typedef enum file_type_t {
    FT_NUKNOWN, FT_TTY, FT_FILE, FT_DIR
} file_type_t;

typedef struct FILE {
    char name[FILE_NAME_SIZE];
    file_type_t type;
    uint32_t size;
    uint32_t ref;
    uint8_t devid;
    uint32_t position;
    uint8_t mode;
} FILE;

FILE *fopen(const char *filepath, const char *mode);
int fgets(FILE *file, char *buf, uint32_t size);
int fclose(FILE *file);

#endif