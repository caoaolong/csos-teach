#include <csos/syscall.h>

int main(int argc, char *argv[])
{
    printf("\e[10;10fHello,%s\e[0f%d\n", "CSOS", 20250101);
    while (TRUE) sleep(1000);
    return 0;
}