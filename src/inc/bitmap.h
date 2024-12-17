#ifndef CSOS_BITMAP_H
#define CSOS_BITMAP_H

#include <kernel.h>

typedef struct bitmap_t
{
    uint32_t bit_size;
    uint8_t *bits;
} bitmap_t;

// 计算位图大小所对应的字节数
static inline uint32_t bitmap_bit_size(uint32_t bit_size)
{
    return (bit_size + 8 - 1) / 8;
}

static inline BOOL bitmap_get_bit(bitmap_t *bitmap, uint32_t index)
{
    BOOL result = bitmap->bits[index / 8] >> (index % 8) & 0x1;
    return result;
}

void bitmap_init(bitmap_t *bitmap, uint8_t *bits, uint32_t bit_size, BOOL value);

int bitmap_alloc_bits(bitmap_t *bitmap, BOOL value, uint32_t size);

void bitmap_set_bits(bitmap_t *bitmap, uint32_t index, uint32_t size, BOOL value);

BOOL bitmap_is_set(bitmap_t *bitmap, uint32_t index);

#endif