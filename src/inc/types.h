#ifndef CSOS_TYPES_H
#define CSOS_TYPES_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// ELF相关数据类型
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef uint16_t fat_cluster_t;

#define MAC_LEN     6
#define IPV4_LEN    4

typedef uint8_t mac_addr[MAC_LEN];
typedef uint8_t ip_addr[IPV4_LEN];

#define NULL    ((void*)0)

#define BOOL    _Bool

#define TRUE    1
#define FALSE   0

#define FILE_NAME_SIZE          32
#define TTY_DEV_NR              8

#define KEY_UP              0x48
#define KEY_DOWN            0x50
#define KEY_LEFT            0x4B
#define KEY_RIGHT           0x4D

#define HLT __asm__ volatile("hlt")

#endif