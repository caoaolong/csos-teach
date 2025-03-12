#include <fs.h>
#include <logf.h>
#include <device.h>
#include <task.h>
#include <csos/string.h>

static list_t mounted_list;
static list_t free_list;

static fs_t *rootfs;
static fs_t *devfs;
static fs_t fs_table[FS_TABLE_SIZE];

static void mount_list_init()
{
    list_init(&free_list);
    for (int i = 0; i < FS_TABLE_SIZE; i++) {
        list_insert_front(&free_list, &fs_table[i].node);
    }
    list_init(&mounted_list);
}

extern fs_op_t devfs_op;
extern fs_op_t fatfs_op;

static fs_op_t *get_fs_op(fs_type_t type, int major)
{
    switch(type) {
    case FS_DEV:
        return &devfs_op;
    case FS_FAT:
        return &fatfs_op;
    default:
        return NULL;
    }
}

static fs_t *mount(fs_type_t type, char *mp, int dev_major, int dev_minor)
{
    fs_t *fs = NULL;
    logf("mount dev: %x:%x to point %s", dev_major, dev_minor, mp);
    list_node_t *pnode = list_get_first(&mounted_list);
    while (pnode) {
        fs = struct_from_field(pnode, fs_t, node);
        if (kernel_strncmp(fs->mp, mp, MOUNT_POINT_SIZE)) {
            logf("%s already mounted", mp);
            return NULL;
        }
        pnode = list_get_next(pnode);
    }
    list_node_t *fnode = list_remove_front(&free_list);
    if (!fnode) {
        logf("free mount list is full");
        list_insert_front(&free_list, fnode);
        return NULL;
    }
    fs = struct_from_field(fnode, fs_t, node);
    kernel_memset(fs, 0, sizeof(fs_t));
    kernel_strcpy(fs->mp, mp);
    fs->op = get_fs_op(type, dev_major);
    if (fs->op->mount(fs, dev_major, dev_minor) < 0) {
        logf("mount failed");
        list_insert_front(&free_list, fnode);
        return NULL;
    }
    list_insert_back(&mounted_list, &fs->node);
    return fs;
}

static void fs_lock(fs_t *fs)
{
    if (fs->mutex) mutex_lock(fs->mutex);
}

static void fs_unlock(fs_t *fs)
{
    if (fs->mutex) mutex_unlock(fs->mutex);
}

void fs_init()
{
    mount_list_init();
    devfs = mount(FS_DEV, "/dev", 0, 0);
    rootfs = mount(FS_FAT, "/", ROOT_DEV);
}

int fs_fopen(FILE *file, char *filepath, const char *mode)
{
    if (!kernel_strncmp(filepath, "/dev/", 5)) {
        fs_lock(devfs);
        if (devfs->op->fopen(devfs, file, filepath + 5, mode) < 0) {
            fs_unlock(devfs);
            return -1;
        }
        fs_unlock(devfs);
    } else {
        fs_lock(rootfs);
        if (rootfs->op->fopen(rootfs, file, filepath, mode) < 0) {
            fs_unlock(rootfs);
            return -1;
        }
        fs_unlock(rootfs);
    }
    file->fd = task_alloc_fd(file);
    return 0;
}

int fs_fread(FILE *file, char *buf, int size)
{
    if (file->type == FT_TTY) {
        fs_lock(devfs);
        if (devfs->op->fread(rootfs, file, buf, size) < 0) {
            fs_unlock(devfs);
            return -1;
        }
        fs_unlock(devfs);
    } else {
        fs_lock(rootfs);
        if (rootfs->op->fread(rootfs, file, buf, size) < 0) {
            fs_unlock(rootfs);
            return -1;
        }
        fs_unlock(rootfs);
    }
    return 0;
}

int fs_fwrite(FILE *file, char *buf, int size)
{
    if (file->type == FT_TTY) {
        fs_lock(devfs);
        int ret = devfs->op->fwrite(devfs, file, buf, size);
        fs_unlock(devfs);
        return ret;
    } else {
        fs_lock(rootfs);
        int ret = rootfs->op->fwrite(rootfs, file, buf, size);
        fs_unlock(rootfs);
        return ret;
    }
}

int fs_lseek(FILE *file, int pos, int dir)
{
    fs_lock(rootfs);
    int ret = rootfs->op->lseek(rootfs, file, pos, dir);
    fs_unlock(rootfs);
    return ret;
}

int fs_remove(char *path)
{
    fs_lock(rootfs);
    int ret = rootfs->op->remove(rootfs, path);
    fs_unlock(rootfs);
    return ret;
}

int fs_fclose(FILE *file)
{
    fs_lock(rootfs);
    rootfs->op->fclose(rootfs, file);
    fs_unlock(rootfs);
    return 0;
}

int fs_isatty(int file)
{
    return 0;
}

int fs_fstat(int file)
{
    return 0;
}

char *fs_sbrk(int size)
{
    return NULL;
}

int fs_opendir(const char *path, DIR *dir)
{
    fs_lock(rootfs);
    int err = rootfs->op->opendir(rootfs, path, dir);
    fs_unlock(rootfs);
    return err;
}

int fs_readdir(DIR *dir, struct dirent *dirent)
{
    fs_lock(rootfs);
    int err = rootfs->op->readdir(rootfs, dir, dirent);
    fs_unlock(rootfs);
    return err;
}

int fs_closedir(DIR *dir)
{
    fs_lock(rootfs);
    int err = rootfs->op->closedir(rootfs, dir);
    fs_unlock(rootfs);
    return err;
}

int fs_getcwd(char *buf)
{
    fs_lock(rootfs);
    int err = rootfs->op->getcwd(rootfs, buf);
    fs_unlock(rootfs);
    return err;
}

int fs_chdir(const char *path)
{
    fs_lock(rootfs);
    int err = rootfs->op->chdir(rootfs, path);
    fs_unlock(rootfs);
    return err;
}

int fs_mkdir(char *path)
{
    fs_lock(rootfs);
    int err = rootfs->op->mkdir(rootfs, path);
    fs_unlock(rootfs);
    return err;
}

int fs_rmdir(char *path)
{
    fs_lock(rootfs);
    int err = rootfs->op->rmdir(rootfs, path);
    fs_unlock(rootfs);
    return err;
}

char fs_getc(FILE *file)
{
    fs_lock(devfs);
    char ch;
    if (devfs->op->fread(rootfs, file, &ch, 1) < 0) {
        logf("getc failed");
        return -1;
    }
    fs_unlock(devfs);
    return ch;
}

void fs_putc(FILE *file)
{

}