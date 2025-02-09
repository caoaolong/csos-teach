#include <device.h>
#include <disk.h>
#include <logf.h>
#include <interrupt.h>
#include <pic.h>
#include <csos/string.h>

static disk_t disks[DISK_PER_CHANNEL];

static sem_t sem;

static void disk_cmd(disk_t *disk, uint32_t ss, uint32_t sc, uint8_t cmd)
{
    outb(DISK_DRIVE(disk), DISK_DRIVE_BASE | disk->drive);
    outb(DISK_SC(disk), (uint8_t)(sc >> 8));
    outb(DISK_LBA_L(disk), (uint8_t)(ss >> 24));
    outb(DISK_LBA_M(disk), 0);
    outb(DISK_LBA_H(disk), 0);

    outb(DISK_SC(disk), (uint8_t)sc);
    outb(DISK_LBA_L(disk), (uint8_t)ss);
    outb(DISK_LBA_M(disk), (uint8_t)(ss >> 8));
    outb(DISK_LBA_H(disk), (uint8_t)(ss >> 16));

    outb(DISK_CMD(disk), cmd);
}

static void disk_read_data(disk_t *disk, void *buf, int size)
{
    uint16_t *c = (uint16_t*)buf;
    for (int i = 0; i < size / 2; i++) {
        *c++ = inw(DISK_DATA(disk));
    }
}

static void disk_write_data(disk_t *disk, void *buf, int size)
{
    uint16_t *c = (uint16_t*)buf;
    for (int i = 0; i < size / 2; i++) {
        outw(DISK_DATA(disk), *c++);
    }
}

static int disk_wait_data(disk_t *disk)
{
    uint8_t status;
    do {
        status = inb(DISK_STATUS(disk));
        if ((status & (DISK_STATUS_BUSY | DISK_STATUS_DQR | DISK_STATUS_ERR)) != DISK_STATUS_BUSY)
            break;
    } while(TRUE);
    return (status & DISK_STATUS_ERR) ? -1 : 0;
}

static char *set_part_name(disk_t *disk, disk_part_t *part, uint8_t i)
{
    int len = kernel_strlen(disk->name);
    kernel_memset(part->name, 0, sizeof(part->name));
    kernel_strcpy(part->name, disk->name);
    part->name[len] = i + '0';
}

static int parse_disk_parts(disk_t *disk)
{
    mbr_t mbr;
    disk_cmd(disk, 0, 1, DISK_CMD_READ);
    int err = disk_wait_data(disk);
    if (err < 0) {
        logf("read mbr failed");
        return -1;
    }
    disk_read_data(disk, &mbr, sizeof(mbr_t));
    for (int i = 0; i < MBR_PRIMARY_PART_NR; i++) {
        disk_part_t *part = &disk->parts[i];
        part_item_t *item = &mbr.part_items[i];
        part->type = item->system_id;
        if (part->type == FS_INVALID) {
            part->total_sector = part->start_sector = 0;
            part->disk = NULL;
        } else {
            set_part_name(disk, part, i);
            part->start_sector = item->relative_sectors;
            part->total_sector = item->total_sectors;
            part->disk = disk;
        }
    }
    return 0;
}

static int identify_disk(disk_t *disk)
{
    disk_cmd(disk, 0, 0, DISK_CMD_IDENTIFY);
    int err = inb(DISK_STATUS(disk));
    if (err == 0) {
        logf("%s doesn't exist", disk->name);
        return -1;
    }
    err = disk_wait_data(disk);
    if (err < 0) {
        logf("%s read falied", disk->name);
        return -1;
    }
    uint16_t buf[256];
    disk_read_data(disk, buf, sizeof(buf));
    disk->sc = *(uint32_t*)(buf + 100);
    disk->sz = DISK_SECTOR_SIZE;
    err = parse_disk_parts(disk);
    if (err < 0) {
        logf("%s parse falied", disk->name);
        return -1;
    }
    return 0;
}

static void print_disk_part(disk_t *disk)
{
    logf(disk->name);
    logf("\tport base: %#x", disk->port_base);
    logf("\ttotal size: %d", disk->sc * disk->sz / 1024 / 1024);
    logf("\tpartition info:");
    for (int i = 0; i < MBR_PRIMARY_PART_NR; i++) {
        disk_part_t *part = &disk->parts[i];
        if (part->type == FS_INVALID)
            continue;
        logf("\t\t%s:", part->name);
        logf("\t\ttype: %d, start sector: %u, total count: %u",
            part->type, part->start_sector, part->total_sector);
    }
}

void disk_init()
{
    sem_init(&sem, 0);
    char disk_name[] = "sd*";
    kernel_memset(disks, 0, sizeof(disks));
    for (int i = 0; i < DISK_PER_CHANNEL; i++) {
        disk_t *disk = &disks[i];
        disk_name[2] = 'a' + i;
        kernel_strcpy(disk->name, disk_name);
        disk->drive = i == 0 ? DISK_MASTER : DISK_SLAVE;
        disk->port_base = IO_BASE_PRIMARY;
        mutex_init(&disk->mutex);
        if (!identify_disk(disk)) {
            print_disk_part(disk);
        }
    }
    install_interrupt_handler(IRQ6_HDC, (uint32_t)interrupt_handler_hdc);
}

// 打开设备
int dev_disk_open(device_t *dev)
{
    int disk_idx = (dev->minor >> 4) & 0xF;
    int part_idx = dev->minor & 0xF;
    if (disk_idx >= DISK_PER_CHANNEL || part_idx >= MBR_PRIMARY_PART_NR) {
        logf("device minor invalid");
        return -1;
    }
    disk_t *disk = &disks[disk_idx];
    if (disk->sc == 0) {
        logf("disk invalid");
        return -1;
    }
    disk_part_t *part = &disk->parts[part_idx];
    if (part->total_sector == 0) {
        logf("disk partition invalid");
        return -1;
    }
    dev->data = part;
    irq_enable(IRQ6_HDC);
    return 0;
}

void handler_hdc(interrupt_frame_t* frame)
{
    sem_notify(&sem);
    send_eoi(IRQ6_HDC);
}

// 读取设备
int dev_disk_read(device_t *dev, int addr, char *buf, int size)
{
    disk_part_t *part = (disk_part_t*)dev->data;
    if (!part) {
        logf("disk partition invalid");
        return -1;
    }
    disk_t *disk = part->disk;
    if (!disk) {
        logf("disk invalid");
        return -1;
    }
    mutex_lock(&disk->mutex);
    dev_disk_command(dev, DISK_CMD_READ, addr, size);
    for (int i = 0; i < size; i++) {
        sem_wait(&sem);
        int err = disk_wait_data(disk);
        if (err < 0) {
            logf("disk read failed");
            break;
        }
        disk_read_data(disk, buf, disk->sz);
    }
    mutex_unlock(&disk->mutex);
    return 0;
}

// 写入到设备
int dev_disk_write(device_t *dev, int addr, char *buf, int size)
{
    disk_part_t *part = (disk_part_t*)dev->data;
    if (!part) {
        logf("disk partition invalid");
        return -1;
    }
    disk_t *disk = part->disk;
    if (!disk) {
        logf("disk invalid");
        return -1;
    }
    mutex_lock(&disk->mutex);
    dev_disk_command(dev, DISK_CMD_WRITE, addr, size);
    for (int i = 0; i < size; i++) {
        disk_write_data(disk, buf, disk->sz);
        sem_wait(&sem);
        int err = disk_wait_data(disk);
        if (err < 0) {
            logf("disk read failed");
            break;
        }
    }
    mutex_unlock(&disk->mutex);
    return 0;
}

// 向设备发送命令
int dev_disk_command(device_t *dev, int cmd, int arg0, int arg1)
{
    disk_part_t *part = (disk_part_t*)dev->data;
    disk_cmd(part->disk, part->start_sector + arg0, arg1, cmd);
    return 0;
}

// 关闭设备
void dev_disk_close(device_t *dev)
{
    dev->data = NULL;
    irq_disable(IRQ6_HDC);
}

device_handle_t device_disk = {
    .name = "disk",
    .major = DEV_DISK,
    .open = dev_disk_open,
    .read = dev_disk_read,
    .write = dev_disk_write,
    .command = dev_disk_command,
    .close = dev_disk_close
};