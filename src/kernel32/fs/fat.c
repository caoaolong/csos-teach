#include <fs.h>
#include <device.h>
#include <logf.h>
#include <disk.h>
#include <task.h>
#include <csos/memory.h>
#include <csos/string.h>

static BOOL fs_bread(fs_fat_t *fat, int sector)
{
    if (sector == fat->pcs) 
        return TRUE;
    if (!device_read(fat->fs->devid, sector, fat->buf, 1)) {
        fat->pcs = sector;
        return TRUE;
    }
    return FLASE;
}

static fat_dir_t *read_fat_dir(fs_fat_t *fat, int index)
{
    if (index < 0 || index >= fat->root_total) 
        return NULL;

    int offset = index * sizeof(fat_dir_t);
    int sector = fat->root_start + offset / fat->bps;
    if (!fs_bread(fat, sector)) 
        return NULL;
    return (fat_dir_t*)(fat->buf + offset % fat->bps);
}

static file_type_t read_fat_ftype(fat_dir_t *fdir)
{
    file_type_t type = FT_NUKNOWN;
    if (fdir->attr & (FDA_VOLID | FDA_HIDDEN | FDA_SYSTEM))
        return type;
    if (fdir->attr & FDA_LNAME == FDA_LNAME)
        return type;
    
    return fdir->attr & FDA_DIRECT ? FT_DIR : FT_FILE;
}

static void read_fat_fname(fat_dir_t *fdir, char *dst)
{
    kernel_memset(dst, 0, FAT_FILE_NAME_SIZE + 1);
    char *c = dst, *ext = NULL;
    for (int i = 0; i < FAT_FILE_NAME_SIZE; i++) {
        if (fdir->name[i] != ' ')
            *c++ = fdir->name[i];
        if (i == 7) {
            ext = c;
            *c++ = '.';
        }
    }

    if (ext && (ext[1] == '\0'))
        ext[0] = '\0';
}

static void fat_to_lfn(char *dst, const char *src)
{
    kernel_memset(dst, ' ', FAT_FILE_NAME_SIZE);
    char *pc = dst;
    for (int i = 0; i < FAT_FILE_NAME_SIZE; i++) {
        if (i == 7) {
            *pc++ = '.';
        }
        char c = *(src + i);
        switch (c) {
            case ' ':
                break;
            default:
                *pc++ = c - 'A' + 'a';
                break;
        }
    }
    if (*(pc - 1) == '.') {
        *(pc - 1) = 0;
    }
}

static void fat_to_sfn(char *dst, const char *src)
{
    kernel_memset(dst, ' ', FAT_FILE_NAME_SIZE);
    char *pc = dst;
    char *pe = dst + FAT_FILE_NAME_SIZE;
    while (*src && (pc < pe)) {
        char c = *src++;
        switch (c) {
            case '/':
                return;
            case '.':
                pc = dst + 8;
                break;
            default:
                if (c >= 'a' && c <= 'z') {
                    c = c - 'a' + 'A';
                }
                *pc++ = c;
                break;
        }
    }
}

static BOOL match_fat_name(fat_dir_t *fdir, const char *name)
{
    char buf[FAT_FILE_NAME_SIZE];
    fat_to_sfn(buf, name);
    return !kernel_memcmp(buf, fdir->name, FAT_FILE_NAME_SIZE);
}

static void read_fat_file(fs_fat_t *fat, FILE *file, fat_dir_t *pdir, int pindex) 
{
    file->type = read_fat_ftype(pdir);
    file->size = pdir->size;
    file->offset = 0;
    file->pindex = pindex;
    file->sblk = (pdir->cluster_h << 16) | pdir->cluster_l;
    file->cblk = file->sblk;
}

static BOOL fat_verify_cluster(fat_cluster_t cluster)
{
    return (cluster < FAT_CLUSTER_INVALID) && (cluster >= 0x2);
}

static fat_cluster_t fat_next_cluster(fs_fat_t *fat, fat_cluster_t cc)
{
    if (!fat_verify_cluster(cc)) 
        return FLASE;
    // 当前簇在FAT表中的字节偏移量
    int offset = cc * sizeof(fat_cluster_t);
    // 当前簇在FAT表中的扇区偏移量
    int sector = offset / fat->bps;
    if (sector >= fat->fat_sectors) {
        return FAT_CLUSTER_INVALID;
    }
    // 当前簇所在扇区的偏移量
    int os = offset % fat->bps;
    if (!fs_bread(fat, fat->fat_start + sector)) {
        return FAT_CLUSTER_INVALID;
    }
    return *(fat_cluster_t *)(fat->buf + os);
}

static BOOL fat_move_file_offset(fs_fat_t *fat, FILE *file, uint32_t cpb, uint32_t mbytes, int expand)
{
    uint32_t co = file->offset % cpb;
    if (co + mbytes >= cpb) {
        // 获取簇链中的下一个簇号
        fat_cluster_t nc = fat_next_cluster(fat, file->cblk);
        if (nc >= FAT_CLUSTER_INVALID)
            return FLASE;
        file->cblk = nc;
    }

    file->offset += mbytes;
    return TRUE;
}

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
    fat->root_start = fat->fat_start + fat->fat_sectors * fat->fat_total;
    fat->data_start = fat->root_start + fat->root_total * 32 / DISK_SECTOR_SIZE;
    fat->fs = fs;
    fat->pcs = -1;
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

int fat_fs_open(fs_t *fs, FILE *file, const char *path, const char *mode)
{
    fs_fat_t *fat = (fs_fat_t *)fs->data;
    fat_dir_t *pdir = NULL;
    int pindex = 0;
    for (int i = 0; i < fat->root_total; i++) {
        fat_dir_t *dir = read_fat_dir(fat, i);
        if (!dir) return -1;
        if (dir->name[0] == FDN_END) break;
        if (dir->name[0] == FDN_FREE) continue;
        if (match_fat_name(dir, path)) {
            pdir = dir;
            pindex = i;
            break;
        }
    }
    if (!pdir) return -1;
    
    read_fat_file(fat, file, pdir, pindex);
    return 0;
}

int fat_fs_read(fs_t *fs, FILE *file, char *buf, int size)
{
    fs_fat_t *fat = (fs_fat_t *)fs->data;
    uint32_t nbytes = size;
    // 判斷要读取的大小是否超过了文件大小
    if (file->offset + nbytes > file->size) {
        // 超过则读取剩余字节数
        nbytes = file->size - file->offset;
    }
    // 每簇的字节数
    uint32_t cpb = fat->bps * fat->spc;
    // 总共读取的字节数
    uint32_t total = 0;
    while (nbytes > 0) {
        // 本次读取的字节数
        uint32_t cr = nbytes;
        // 本次读取的偏移量
        uint32_t co = file->offset % cpb;
        // 文件数据所在起始扇区号
        uint32_t ss = fat->data_start + (file->cblk - 2) * fat->spc;
        if ((co == 0) && (nbytes == cpb)) {
            if (device_read(fat->fs->devid, ss, fat->buf, fat->spc) < 0) {
                return total;
            }
            cr = cpb;
        } else {
            // 判断要读取的大小是否超过当前簇大小
            if (co + cr > cpb) {
                // 超过则读取当前簇剩余的字节数
                cr = cpb - co;
            }
            fat->pcs = -1;
            if (device_read(fat->fs->devid, ss, fat->buf, fat->spc) < 0) {
                return total;
            }
            kernel_memcpy(buf, fat->buf + co, cr);
        }
        buf += cr;
        nbytes -= cr;
        total += cr;
        if (!fat_move_file_offset(fat, file, cpb, cr, 0)) {
            return total;
        }
    }
    return total;
}

int fat_fs_write(fs_t *fs, FILE *file, char *buf, int size)
{
    return device_write(file->devid, file->offset, buf, size);
}

void fat_fs_close(fs_t *fs, FILE *file)
{
    device_close(file->devid);
}

int fat_fs_seek(fs_t *fs, FILE *file, uint32_t offset, int dir)
{
    return 0;
}

int fat_fs_stat(fs_t *fs, FILE *file, stat_t *st)
{
    return 0;
}

int fat_fs_opendir(fs_t *fs, const char *path, DIR *dir)
{
    dir->index = 0;
    return 0;
}

int fat_fs_readdir(fs_t *fs, DIR *dir, struct dirent *dirent)
{
    fs_fat_t *fat = (fs_fat_t *)fs->data;
    while (dir->index < fat->root_total) {
        fat_dir_t *fdir = read_fat_dir(fat, dir->index);
        if (fdir == NULL) return -1;
        if (fdir->name[0] == FDN_END) break;
        if (fdir->name[0] != FDN_FREE) {
            file_type_t type = read_fat_ftype(fdir);
            if (type == FT_DIR || type == FT_FILE) {
                dirent->d_reclen = fdir->size;
                dirent->d_type = type;
                read_fat_fname(fdir, dirent->d_name);
                dirent->d_ino = dir->index++;
                return 0;
            }
        }
        dir->index++;
    }
    return -1;
}

int fat_fs_closedir(fs_t *fs, DIR *dir)
{
    return 0;
}

static int fat_fs_init_task_wd(fs_fat_t *fat, task_t *task)
{
    task->wd.sector = fat->root_start;
    task->wd.offset = 0;
    return 0;
}

static int fat_fs_read_path(fs_fat_t *fat, int sector, int offset, char *buf)
{
    if (sector == fat->root_start && offset == 0) {
        kernel_strcpy(buf, "/");
        return 0;
    }
    char nbuf[11];
    char *pbuf = buf;
    uint32_t srclen = 0;
    // 保存上一次的簇号
    uint32_t lcnum = 0;
    uint32_t dcount = fat->bps * fat->spc / sizeof(fat_dir_t);
    uint32_t sc = fat->root_start;
    fat_dir_t dirs[dcount];
    // 读取当前目录信息
    if (device_read(fat->fs->devid, sector, (char *)&dirs, 1) < 0) {
        logf("disk read failed!");
        return -1;
    }
    fat_dir_t *pdir = &dirs[offset];
    fat_to_lfn(nbuf, pdir->name);
    srclen = kernel_strlen(nbuf);
    kernel_reverse_strcpy(pbuf, nbuf, srclen);
    pbuf += srclen;
    // 读取父级目录信息
    do {
        lcnum = sector;
        // 读取[..]目录
        pdir = &dirs[1];
        sector = (pdir->cluster_h << 16) | pdir->cluster_l - 2;
        if (sector < 0) {
            // 表示到达根目录区
            sector = fat->root_start;
        } else {
            sector += fat->data_start;
        }
        if (device_read(fat->fs->devid, sector, (char *)&dirs, 1) < 0) {
            logf("disk read failed!");
            return -1;
        }
        for (int i = 0; i < dcount; i++) {
            pdir = &dirs[i];
            if (pdir->name[0] == FDN_END) break;
            if (pdir->name[0] == FDN_FREE) continue;
            uint32_t cluster = fat->data_start + (pdir->cluster_h << 16) | pdir->cluster_l - 2;
            if (cluster == lcnum) {
                fat_to_lfn(nbuf, pdir->name);
                srclen = kernel_strlen(nbuf);
                *pbuf = '/';
                pbuf++;
                kernel_reverse_strcpy(pbuf, nbuf, srclen);
                pbuf += srclen;
                break;
            }
        }
    } while (sector > fat->root_start);
    *pbuf = '/';
    pbuf++;
    *pbuf = 0;
    kernel_reverse_str(buf);
    return 0;
}

int fat_fs_getcwd(fs_t *fs, char *buf)
{
    fs_fat_t *fat = (fs_fat_t *)fs->data;
    task_t *task = get_running_task();
    int err = 0;

    if (task->wd.sector < 0 || task->wd.offset < 0) {
        err = fat_fs_init_task_wd(fat, task);
        if (err < 0) return -1;
    }

    return fat_fs_read_path(fat, task->wd.sector, task->wd.offset, buf);
}

int fat_fs_chdir(fs_t *fs, const char *path)
{
    fs_fat_t *fat = (fs_fat_t *)fs->data;
    task_t *task = get_running_task();

    char buf[10];
    int si = 0, di = 0;

    uint32_t dcount = fat->bps * fat->spc / sizeof(fat_dir_t);
    uint32_t sc = fat->root_start;
    fat_dir_t dirs[dcount];
    BOOL found = FLASE;
    while (sc < FAT_CLUSTER_INVALID && *(path + si++) != 0) {
        kernel_memset(buf, 0, sizeof(buf));
        do {
            buf[di] = *(path + si);
            di++;
            si++;
        } while(*(path + si) != '/' && *(path + si) != 0);

        if (device_read(fs->devid, sc, (char *)&dirs, 1) < 0) {
            logf("device read failed!");
            return -1;
        }
        for (int i = 0; i < dcount; i++) {
            fat_dir_t *dir = &dirs[i];
            if (dir->name[0] == FDN_END) break;
            if (dir->name[0] == FDN_FREE) continue;
            if (match_fat_name(dir, buf)) {
                task->wd.sector = sc;
                task->wd.offset = i;
                sc = fat->data_start + ((dir->cluster_h << 16) | dir->cluster_l) - 2;
                found = TRUE;
                break;
            }
        }
        di = 0;
        if (found) {
            continue;
        } else {
            sc = (uint32_t)fat_next_cluster(fat, (fat_cluster_t)sc);
        }
    }

    return 0;
}

fs_op_t fatfs_op = {
    .mount = fat_fs_mount,
    .unmount = fat_fs_unmount,
    .fopen = fat_fs_open,
    .fread = fat_fs_read,
    .fwrite = fat_fs_write,
    .fclose = fat_fs_close,
    .lseek = fat_fs_seek,
    .fstat = fat_fs_stat,
    .opendir = fat_fs_opendir,
    .readdir = fat_fs_readdir,
    .closedir = fat_fs_closedir,
    .getcwd = fat_fs_getcwd,
    .chdir = fat_fs_chdir
};