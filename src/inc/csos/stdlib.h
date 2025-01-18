#ifndef CSOS_STDLIB_H
#define CSOS_STDLIB_H

#include <kernel.h>

uint8_t bcd_to_bin(uint8_t value);

uint8_t bin_to_bcd(uint8_t value);

int strings_count(char *argv[]);

char *get_file_name(char *name);

#endif