#include <csos/syscall.h>

int main(int argc, char *argv[])
{
    for(int i = 0; i < 1000; i++) {
        printf("Hello,%s\n%d\n", "CSOS", 20250101);
    }
    while (TRUE) sleep(1000);
    return 0;
}