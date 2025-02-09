#include <fs.h>
#include <device.h>

int dev_fs_mount(fs_t *fs, int major, int minor)
{
    fs->type = FS_DEV;
    return 0;
}

void dev_fs_unmount(fs_t *fs)
{

}

int dev_fs_open(fs_t *fs, const char *path, file_t *file)
{
    return 0;
}

int dev_fs_read(char *buf, int size, file_t *file)
{
    return device_read(file->devid, file->position, buf, size);
}

int dev_fs_write(char *buf, int size, file_t *file)
{
    return device_write(file->devid, file->position, buf, size);
}

void dev_fs_close(file_t *file)
{
    device_close(file->devid);
}

int dev_fs_seek(file_t *file, uint32_t offset, int dir)
{
    return 0;
}

int dev_fs_stat(file_t *file, stat_t *st)
{
    return 0;
}

fs_op_t devfs_op = {
    .mount = dev_fs_mount,
    .unmount = dev_fs_unmount,
    .open = dev_fs_open,
    .read = dev_fs_read,
    .write = dev_fs_write,
    .close = dev_fs_close,
    .seek = dev_fs_seek,
    .stat = dev_fs_stat
};