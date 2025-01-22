#include <csos/syscall.h>

int main(int argc, char *argv[])
{
    char *c = (char*)malloc(5000);
    while (TRUE) sleep(1000);
    return 0;
}