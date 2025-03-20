#include <csos/string.h>

void strcpy(char *dst, const char *src)
{
    if (!src) return;

    while (*src) 
        *dst++ = *src++;

    *dst = '\0';
}

void reverse_strcpy(char *dst, const char *src, uint32_t srclen)
{
    if (!src) return;
    const char *p = src + srclen - 1;
    while (p >= src) 
        *dst++ = *p--;

    *dst = '\0';
}

void reverse_str(char *str)
{
    char c;
    char *pb = str;
    char *pe = str + strlen(str) - 1;
    while (pb < pe) {
        // 交换字符
        c = *pb; *pb = *pe; *pe = c;
        pb++;
        pe--;
    }
}

void strncpy(char *dst, const char *src, uint32_t size)
{
    if (!dst || !src || !size) return;

    char *d = dst;
    const char *s = src;
    while ((size -- > 0) && (*s)) 
        *dst++ = *src++;

    if (size == 0) *(d - 1) = '\0'; else *d = '\0';
}

int strcmp(const char *str1, const char *str2)
{
    if (!str1 || !str2) return -1;

    while (*str1 && *str2 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }

    return !(*str1 == *str2);
}

int strncmp(const char *str1, const char *str2, uint32_t size)
{
    if (!str1 || !str2 || !size) return -1;

    while (*str1 && *str2 && (*str1 == *str2) && size)
    {
        str1++;
        str2++;
    }

    return !((*str1 == '\0') || (*str2 == '\0') || (*str1 == *str2));
}

uint32_t strlen(const char *str)
{
    if (!str) return 0;

    const char *c = str;
    uint32_t length = 0;
    while (*c++) 
        length++;

    return length;
}

void memcpy(void *dst, void *src, uint32_t size)
{
    if (!dst || !src || !size) return;

    uint8_t *s = (uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;
    while (size--) 
        *d++ = *s++;
}

void memset(void *dst, uint8_t value, uint32_t size)
{
    if (!dst || !size) return;
    uint8_t *d = (uint8_t *)dst;
    while (size--) 
        *d++ = value;
}

int memcmp(void *v1, void *v2, uint32_t size)
{
    if (!v1 || !v2 || !size) return -1;

    uint8_t *p1 = (uint8_t *)v1;
    uint8_t *p2 = (uint8_t *)v2;
    while (size--)
        if (*p1++ != *p2++) return 1;
    
    return 0;
}