#include <csos/syscall.h>
#include <fs/dir.h>

DIR *opendir(const char *path)
{
    DIR *dir = (DIR *)malloc(sizeof(DIR));
    if (!dir) return NULL;

    syscall_arg_t args = {SYS_NR_OPENDIR, (int)path, (int)dir, 0, 0};
    int err = _syscall(&args);
    if (err < 0) {
        // TODO: free
        return NULL;
    }
    return dir;
}

struct dirent *readdir(DIR *dir)
{
    syscall_arg_t args = {SYS_NR_READDIR, (int)dir, (int)&dir->dirent, 0, 0};
    int err = _syscall(&args);
    if (err < 0) return NULL;
    return &dir->dirent;
}

int closedir(DIR *dir)
{
    syscall_arg_t args = {SYS_NR_CLOSEDIR, (int)dir, 0, 0, 0};
    int err = _syscall(&args);
    // TODO: free
    return err;
}

char *getcwd()
{
    char *buf = (char *)malloc(256);
    syscall_arg_t args = {SYS_NR_GETCWD, (int)buf, 0, 0, 0};
    if (_syscall(&args) < 0) {
        return NULL;
    }
    return buf;
}

int chdir(const char *path)
{
    syscall_arg_t args = {SYS_NR_CHDIR, (int)path, 0, 0, 0};
    return _syscall(&args);
}

int mkdir(const char *path)
{
    syscall_arg_t args = {SYS_NR_MKDIR, (int)path, 0, 0, 0};
    return _syscall(&args);
}

int rmdir(const char *path)
{
    syscall_arg_t args = {SYS_NR_RMDIR, (int)path, 0, 0, 0};
    return _syscall(&args);
}