#include <csos/syscall.h>

extern int main(int argc, char *argv[]);

void cstart(int argc, char *argv[])
{
    exit(main(argc, argv));
}