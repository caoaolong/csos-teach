#ifndef CSOS_FAT_H
#define CSOS_FAT_H

#include <kernel.h>

#define OEM_NAME_SIZE       8
#define VOLUME_NAME_SIZE    11
#define FAT_NAME_SIZE       8

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
} fs_fat_t;

#endif