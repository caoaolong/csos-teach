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

int dev_fs_open(fs_t *fs, FILE *file, const char *path, const char *mode)
{
    return 0;
}

int dev_fs_read(fs_t *fs, FILE *file, char *buf, int size)
{
    return device_read(file->devid, file->position, buf, size);
}

int dev_fs_write(fs_t *fs, FILE *file, char *buf, int size)
{
    return device_write(file->devid, file->position, buf, size);
}

void dev_fs_close(fs_t *fs, FILE *file)
{
    device_close(file->devid);
}

int dev_fs_seek(fs_t *fs, FILE *file, uint32_t offset, int dir)
{
    return 0;
}

int dev_fs_stat(fs_t *fs, FILE *file, stat_t *st)
{
    return 0;
}

fs_op_t devfs_op = {
    .mount = dev_fs_mount,
    .unmount = dev_fs_unmount,
    .fopen = dev_fs_open,
    .fread = dev_fs_read,
    .fwrite = dev_fs_write,
    .fclose = dev_fs_close,
    .lseek = dev_fs_seek,
    .fstat = dev_fs_stat
};