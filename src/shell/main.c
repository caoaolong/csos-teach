#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    fopen("/dev/tty0", "w");    // stdin
    dup(0);                     // stdout
    dup(0);                     // stderr

    printf("Hello,World!\n");
    while(TRUE) {
        char c = getc();
    }
    return 0;
}