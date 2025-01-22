#include <csos/syscall.h>

int main(int argc, char *argv[])
{
    char *c = (char*)malloc(100);
    while (TRUE) sleep(1000);
    return 0;
}