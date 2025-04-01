#ifndef CSOS_TERM_H
#define CSOS_TERM_H

#include <types.h>

typedef struct term_t {
    uint8_t cc, cr;
} term_t;

void clear();

int tcgetattr(int fd, term_t *term);

int tcsetattr(int fd, term_t *term);

#endif