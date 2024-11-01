asm(".code16gcc");
#ifndef CSOS_OS_H
#define CSOS_OS_H

#define BMB __asm__ volatile("xchgw %bx, %bx")

#endif