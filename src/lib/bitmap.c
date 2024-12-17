#include <bitmap.h>
#include <csos/string.h>

void bitmap_init(bitmap_t *bitmap, uint8_t *bits, uint32_t bit_size, BOOL value)
{
    bitmap->bit_size = bit_size;
    bitmap->bits = bits;
    int bytes = bitmap_bit_size(bit_size);
    kernel_memset(bitmap->bits, value, bytes);
}

// 分配连续size个值为value的位
int bitmap_alloc_bits(bitmap_t *bitmap, BOOL value, uint32_t size)
{
    int sidx = 0, ridx = -1;
    while (sidx < bitmap->bit_size)
    {
        if (bitmap_get_bit(bitmap, sidx) != value)
        {
            sidx ++;
            continue;
        }
        ridx = sidx;
        int i;
        for (i = 1; i < size && sidx < bitmap->bit_size; i++)
        {
            if (bitmap_get_bit(bitmap, sidx++) != value)
            {
                ridx = -1;
                break;
            }
        }
        
        if (i >= size) {
            bitmap_set_bits(bitmap, ridx, size, TRUE);
            return ridx;
        }
    }
    return ridx;
}

void bitmap_set_bits(bitmap_t *bitmap, uint32_t index, uint32_t size, BOOL value)
{
    for (int i = 0; i < size && index < bitmap->bit_size; i++)
    {
        if (value) {
            bitmap->bits[index / 8] |= (1 << (index % 8));
        } else {
            bitmap->bits[index / 8] &= ~(1 << (index % 8));
        }
        index ++;
    }
}

BOOL bitmap_is_set(bitmap_t *bitmap, uint32_t index)
{
    return bitmap_get_bit(bitmap, index);
}