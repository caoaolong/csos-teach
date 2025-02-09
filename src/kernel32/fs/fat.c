#include <fs.h>
#include <device.h>
#include <fs/fat.h>
#include <logf.h>
#include <disk.h>
#include <csos/memory.h>
#include <csos/string.h>

int fat_fs_mount(fs_t *fs, int major, int minor)
{
    int devid = device_open(major, minor, NULL);
    if (devid < 0) {
        logf("device open failed");
        return -1;
    }
    boot_record_t *br = (boot_record_t*)alloc_page();
    if (!br) {
        logf("can't alloc page");
        device_close(devid);
        return -1;
    }
    int size = device_read(devid, 0, (char*)br, 1);
    if (size < 0) {
        logf("can't read sector");
        free_page((uint32_t)br);
        device_close(devid);
        return -1;
    }
    fs_fat_t *fat = &fs->fat_data;
    fat->buf = (char*)br;
    fat->bps = br->bytes_per_sector;
    fat->spc = br->sectors_per_cluster;
    fat->fat_start = br->reserved_sectors;
    fat->fat_total = br->noc_fats;
    fat->fat_sectors = br->sectors_per_fat;
    fat->root_total = br->max_rde;
    fat->root_start = fat->fat_start + fat->fat_sectors + fat->fat_total;
    fat->data_start = fat->root_start + fat->root_total * 32 / DISK_SECTOR_SIZE;
    fat->fs = fs;
    if (fat->fat_total != 2 || kernel_memcmp(br->fat_name, "FAT16", 5)) {
        logf("invalid fat format");
        free_page((uint32_t)br);
        device_close(devid);
        return -1;
    }
    fs->type = FS_FAT;
    fs->data = &fs->fat_data;
    fs->devid = devid;
    return 0;
}

void fat_fs_unmount(fs_t *fs)
{
    fs_fat_t *fat = fs->data;
    free_page((uint32_t)fat->buf);
    device_close(fs->devid);
}

int fat_fs_open(fs_t *fs, const char *path, file_t *file)
{
    return 0;
}

int fat_fs_read(char *buf, int size, file_t *file)
{
    return device_read(file->devid, file->position, buf, size);
}

int fat_fs_write(char *buf, int size, file_t *file)
{
    return device_write(file->devid, file->position, buf, size);
}

void fat_fs_close(file_t *file)
{
    device_close(file->devid);
}

int fat_fs_seek(file_t *file, uint32_t offset, int dir)
{
    return 0;
}

int fat_fs_stat(file_t *file, stat_t *st)
{
    return 0;
}

fs_op_t fatfs_op = {
    .mount = fat_fs_mount,
    .unmount = fat_fs_unmount,
    .open = fat_fs_open,
    .read = fat_fs_read,
    .write = fat_fs_write,
    .close = fat_fs_close,
    .seek = fat_fs_seek,
    .stat = fat_fs_stat
};