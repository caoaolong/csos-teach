#include <fs/file.h>
#include <csos/syscall.h>
#include <csos/stdarg.h>

static char buffer[1024];

int dup(int fd)
{
    syscall_arg_t args = {SYS_NR_DUP, fd, 0, 0, 0};
    _syscall(&args);
}

char getc()
{
    syscall_arg_t args = {SYS_NR_GETC, stdin, 0, 0, 0};
    return _syscall(&args);
}

void putc()
{
    syscall_arg_t args = {SYS_NR_PUTC, stdout, 0, 0, 0};
    _syscall(&args);
}

void fprintf(int fd, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buffer, fmt, args);
    va_end(args);
    syscall_arg_t printf_arg = { SYS_NR_PRINTF, fd, (uint32_t)buffer, i, 0 };
    _syscall(&printf_arg);
}

void printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buffer, fmt, args);
    va_end(args);
    syscall_arg_t printf_arg = { SYS_NR_PRINTF, stdout, (uint32_t)buffer, i, 0 };
    _syscall(&printf_arg);
}

int fopen(const char *filepath, const char *mode)
{
    FILE *file = (FILE *)malloc(sizeof(FILE));
    syscall_arg_t args = {SYS_NR_FOPEN, (int)file, (int)filepath, (int)mode, 0};
    return _syscall(&args);
}

int fgets(int fd, char *buf, uint32_t size)
{
    syscall_arg_t args = {SYS_NR_FGETS, fd, (int)buf, size - 1, 0};
    int nbytes = _syscall(&args);
    buf[nbytes] = 0;
    return nbytes;
}

int fputs(int fd, char *buf, uint32_t size)
{
    syscall_arg_t args = {SYS_NR_FPUTS, fd, (int)buf, size, 0};
    int nbytes = _syscall(&args);
    return nbytes;
}

int fclose(int fd)
{
    syscall_arg_t args = {SYS_NR_FCLOSE, fd, 0, 0, 0};
    return _syscall(&args);
}

int remove(const char *path)
{
    syscall_arg_t args = {SYS_NR_REMOVE, (int)path, 0, 0, 0};
    return _syscall(&args);
}

int lseek(int fd, int offset, int dir)
{
    syscall_arg_t args = {SYS_NR_LSEEK, fd, offset, dir, 0};
    return _syscall(&args);
}