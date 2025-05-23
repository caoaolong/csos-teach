#ifndef CSOS_STRING_H
#define CSOS_STRING_H

#include <types.h>

void strcpy(char *dst, const char *src);

void reverse_strcpy(char *dst, const char *src, uint32_t srclen);

void reverse_str(char *str);

void strncpy(char *dst, const char *src, uint32_t size);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, uint32_t size);

uint32_t strlen(const char *str);

void memcpy(void *dst, void *src, uint32_t size);

void memset(void *dst, uint8_t value, uint32_t size);

int memcmp(void *v1, void *v2, uint32_t size);

void kernel_strcpy(char *dst, const char *src);

void kernel_reverse_strcpy(char *dst, const char *src, uint32_t srclen);

void kernel_reverse_str(char *str);

void kernel_strncpy(char *dst, const char *src, uint32_t size);

int kernel_strcmp(const char *str1, const char *str2);

int kernel_strncmp(const char *str1, const char *str2, uint32_t size);

uint32_t kernel_strlen(const char *str);

void kernel_memcpy(void *dst, void *src, uint32_t size);

void kernel_memset(void *dst, uint8_t value, uint32_t size);

int kernel_memcmp(void *v1, void *v2, uint32_t size);
#endif