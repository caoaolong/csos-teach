#include <fs.h>
#include <logf.h>
#include <device.h>
#include <csos/string.h>

static list_t mounted_list;
static list_t free_list;

static fs_t *rootfs;
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

void fs_init()
{
    mount_list_init();
    fs_t *devfs = mount(FS_DEV, "/dev", 0, 0);
    rootfs = mount(FS_FAT, "/", ROOT_DEV);
}

int fs_open(const char *name, int flags, ...)
{
    return 0;
}

int fs_read(int file, char *buf, int len)
{
    return 0;
}

int fs_write(int file, char *buf, int len)
{
    return 0;
}

int fs_lseek(int file, int pos, int dir)
{
    return 0;
}

int fs_close(int file)
{
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