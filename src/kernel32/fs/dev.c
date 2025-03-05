#include <fs.h>
#include <device.h>
#include <logf.h>
#include <csos/string.h>

int dev_fs_mount(fs_t *fs, int major, int minor)
{
    fs->type = FS_DEV;
    return 0;
}

void dev_fs_unmount(fs_t *fs)
{

}

int dev_fs_open(fs_t *fs, FILE *file, char *path, const char *mode)
{
    kernel_strcpy(file->name, path);
    file->sblk = file->cblk = file->offset = 0;
    // 设备文件系统devid为0
    file->devid = 0;
    // 设备类型为TTY
    file->type = FT_TTY;
    // dcluster存储屏幕号
    // doffset存储偏移量
    file->dcluster = file->doffset = 0;

    if (*mode == 'w') {
        file->mode = FM_WRITE;
    } else {
        file->mode = FM_READ;
    }
    if (!kernel_strncmp(path, "tty", 3)) {
        int screen = *(path + 3) - '0';
        file->dcluster = screen;
        return device_open(DEV_TTY, screen, NULL);
    }
    return -1;
}

int dev_fs_read(fs_t *fs, FILE *file, char *buf, int size)
{
    return device_read(file->devid, file->offset, buf, size);
}

int dev_fs_write(fs_t *fs, FILE *file, char *buf, int size)
{
    if (file->fd < 0) {
        logf("task file not exists");
        return -1;
    }
    return device_write(file->devid, file->offset, buf, size);
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