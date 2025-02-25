#ifndef CSOS_TYPES_H
#define CSOS_TYPES_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// ELF相关数据类型
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef uint16_t fat_cluster_t;

#define NULL    ((void*)0)

#define BOOL    _Bool

#define TRUE    1
#define FALSE   0

#define FILE_NAME_SIZE  32

#define HLT __asm__ volatile("hlt")

#endif