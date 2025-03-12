#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    fopen("/dev/tty0", "w");
    printf("Hello,World!\n");
    while(TRUE) {
        char c = getc();
    }
    return 0;
}