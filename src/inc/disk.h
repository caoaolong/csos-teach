#ifndef CSOS_DISK_H
#define CSOS_DISK_H

#include <kernel.h>

#define DISK_NAME_SIZE      32
#define DISK_PER_CHANNEL    2
#define DISK_SECTOR_SIZE    512

#define IO_BASE_PRIMARY     0x1F0
#define DISK_DRIVE_BASE     0xE0
#define DISK_DATA(disk)     (disk->port_base + 0)
#define DISK_ERR(disk)      (disk->port_base + 1)
#define DISK_SC(disk)       (disk->port_base + 2)
#define DISK_LBA_L(disk)    (disk->port_base + 3)
#define DISK_LBA_M(disk)    (disk->port_base + 4)
#define DISK_LBA_H(disk)    (disk->port_base + 5)
#define DISK_DRIVE(disk)    (disk->port_base + 6)
#define DISK_STATUS(disk)   (disk->port_base + 7)
#define DISK_CMD(disk)      (disk->port_base + 7)

#define DISK_STATUS_ERR     (1 << 0)
#define DISK_STATUS_DQR     (1 << 3)
#define DISK_STATUS_DF      (1 << 5)
#define DISK_STATUS_BUSY    (1 << 7)

#define DISK_CMD_IDENTIFY   0xEC
#define DISK_CMD_READ       0x24
#define DISK_CMD_WRITE      0x34

typedef enum DRIVE {
    DISK_MASTER = (0 << 4),
    DISK_SLAVE = (1 << 4)
} DRIVE;

typedef struct disk_part_t {

} disk_part_t;

typedef struct disk_t {
    // name
    char name[DISK_NAME_SIZE];
    // sector size
    uint16_t sz;
    // sector count
    uint32_t sc;
    // drive
    DRIVE drive;
    // port base
    uint16_t port_base;
} disk_t;

void disk_init();

#endif