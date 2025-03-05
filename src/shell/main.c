#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    fopen("/dev/tty0", "r");
    printf("Hello,World!\n");
    return 0;
}