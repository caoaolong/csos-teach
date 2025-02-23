#ifndef CSOS_FAT_H
#define CSOS_FAT_H

#include <kernel.h>

#define OEM_NAME_SIZE       8
#define VOLUME_NAME_SIZE    11
#define FAT_FILE_NAME_SIZE  11
#define FAT_NAME_SIZE       8
#define FAT_CLUSTER_INVALID 0xFFF8

typedef struct boot_record_t {
    uint16_t jump_code;
    uint8_t nop;
    uint8_t oem_name[OEM_NAME_SIZE];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t noc_fats;
    uint16_t max_rde;
    uint16_t nosp;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t num_hidden_sectors;
    uint32_t num_sectors;
    uint16_t num_ld;
    uint8_t ext_sign;
    uint32_t num_serial;
    uint8_t volume_name[VOLUME_NAME_SIZE];
    uint8_t fat_name[FAT_NAME_SIZE];
} boot_record_t;

struct fs_t;

typedef struct fs_fat_t {
    uint32_t fat_start;
    uint32_t fat_total;
    uint32_t fat_sectors;
    uint32_t root_start;
    uint32_t root_total;
    uint32_t data_start;
    // bytes per sector
    uint32_t bps;
    // sectors per cluster
    uint32_t spc;
    // fs
    struct fs_t *fs;
    // buf
    char *buf;
    // current sector
    uint32_t pcs;
    // current free cluster;
    int free;
} fs_fat_t;

enum {
    FDA_RDONLY  = 0x1,
    FDA_HIDDEN  = 0x2,
    FDA_SYSTEM  = 0x4,
    FDA_VOLID   = 0x8,
    FDA_DIRECT  = 0x10,
    FDA_ARCHIVE = 0x20,
    FDA_LNAME   = 0xF
};

#define FDN_FREE    0xE5
#define FDN_END     0x00

typedef union fat_date_t {
    uint16_t v;
    struct {
        uint16_t date:5;
        uint16_t month:4;
        uint16_t year:7;
    };
} fat_date_t;

typedef union fat_time_t {
    uint16_t v;
    struct {
        uint16_t second:5;
        uint16_t minute:6;
        uint16_t hour:5;
    };
} fat_time_t;

typedef struct fat_dir_t {
    uint8_t name[FAT_FILE_NAME_SIZE];
    uint8_t attr;
    uint8_t reserved_nt;
    uint8_t crt_hdths;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t la_date;
    uint16_t cluster_h;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t cluster_l;
    uint32_t size;
} fat_dir_t;

#endif