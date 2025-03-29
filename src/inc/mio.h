#ifndef CSOS_MIO_H
#define CSOS_MIO_H

#include <kernel.h>

// 映射内存 IO

// 内存输入字节 8bit
uint8_t minb(uint32_t addr);
// 内存输入字 16bit
uint16_t minw(uint32_t addr);
// 内存输入双字 32bit
uint32_t minl(uint32_t addr);


// 内存输出字节
void moutb(uint32_t addr, uint8_t value);
// 内存输出字
void moutw(uint32_t addr, uint16_t value);
// 内存输出双字
void moutl(uint32_t addr, uint32_t value);

#endif