#include <csos/syscall.h>

int main(int argc, char *argv[])
{
    printf("Hello,%s\n%d", "CSOS", 20250101);
    while (TRUE) sleep(1000);
    return 0;
}