#ifndef CSOS_DISK_H
#define CSOS_DISK_H

#include <kernel.h>
#include <csos/mutex.h>
#include <csos/sem.h>

#define DISK_NAME_SIZE      32
#define PART_NAME_SIZE      32
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
#define MBR_PRIMARY_PART_NR 4

typedef struct part_item_t {
    uint8_t boot_active;
    uint8_t start_header;
    uint16_t start_sector:6;
    uint16_t start_cylinder:10;
    uint8_t system_id;
    uint8_t end_header;
    uint16_t end_sector:6;
    uint16_t end_cylinder:10;
    uint32_t relative_sectors;
    uint32_t total_sectors;
} part_item_t;

typedef struct mbr_t {
    uint8_t code[446];
    part_item_t part_items[MBR_PRIMARY_PART_NR];
    uint8_t boot_sig[2];
} mbr_t;

typedef enum DRIVE {
    DISK_MASTER = (0 << 4),
    DISK_SLAVE = (1 << 4)
} DRIVE;

typedef enum PartType {
    FS_INVALID = 0x0,
    FS_FAT16_0 = 0x6,
    FS_FAT16_1 = 0xE
} PartType;

struct disk_t;
typedef struct disk_part_t {
    char name[PART_NAME_SIZE];
    struct disk_t *disk;
    PartType type;
    uint32_t start_sector;
    uint32_t total_sector;
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
    // mutex
    mutex_t mutex;
    // part info
    disk_part_t parts[MBR_PRIMARY_PART_NR];
} disk_t;

void disk_init();

#endif