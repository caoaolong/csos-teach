#include <csos/syscall.h>
#include <fs/file.h>

FILE *fopen(const char *filepath, const char *mode)
{
    FILE *file = (FILE *)malloc(sizeof(FILE));
    syscall_arg_t args = {SYS_NR_FOPEN, (int)file, (int)filepath, (int)mode, 0};
    int err = _syscall(&args);
    if (err < 0) {
        // TODO: free
        return NULL;
    }
    return file;
}

int fgets(FILE *file, char *buf, uint32_t size)
{
    syscall_arg_t args = {SYS_NR_FGETS, (int)file, (int)buf, size - 1, 0};
    int nbytes = _syscall(&args);
    buf[nbytes] = 0;
    return nbytes;
}

int fclose(FILE *file)
{
    syscall_arg_t args = {SYS_NR_FCLOSE, (int)file, 0, 0, 0};
    return _syscall(&args);
}