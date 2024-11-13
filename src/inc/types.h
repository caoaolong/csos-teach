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

#define NULL    ((void*)0)

#define BOOL    _Bool

#define TRUE    1
#define FLASE   0

#define _packed __attribute__((__packed__))
#define HLT __asm__ volatile("hlt")

#endif