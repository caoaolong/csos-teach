#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    fopen(argv[0], "w");        // stdin
    dup(0);                     // stdout
    dup(0);                     // stderr

    printf("%s\n", argv[0]);
    while(TRUE) {
        char c = getc();
    }
    return 0;
}