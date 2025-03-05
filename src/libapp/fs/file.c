#include <fs/file.h>
#include <csos/syscall.h>
#include <csos/stdarg.h>

static char buffer[1024];
static FILE *stdout = NULL;
static FILE *stdin = NULL;

void printf(const char *fmt, ...)
{
    if (stdout == NULL) {
        stdout = fopen("/dev/tty0", "w");
    }
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buffer, fmt, args);
    va_end(args);
    syscall_arg_t printf_arg = { SYS_NR_PRINTF, (uint32_t)stdout, (uint32_t)buffer, i, 0 };
    _syscall(&printf_arg);
}

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

int fputs(FILE *file, char *buf, uint32_t size)
{
    syscall_arg_t args = {SYS_NR_FPUTS, (int)file, (int)buf, size, 0};
    int nbytes = _syscall(&args);
    return nbytes;
}

int fclose(FILE *file)
{
    syscall_arg_t args = {SYS_NR_FCLOSE, (int)file, 0, 0, 0};
    return _syscall(&args);
}

int remove(const char *path)
{
    syscall_arg_t args = {SYS_NR_REMOVE, (int)path, 0, 0, 0};
    return _syscall(&args);
}

int lseek(FILE *file, int offset, int dir)
{
    syscall_arg_t args = {SYS_NR_LSEEK, (int)file, offset, dir, 0};
    return _syscall(&args);
}