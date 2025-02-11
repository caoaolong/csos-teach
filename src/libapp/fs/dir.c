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
