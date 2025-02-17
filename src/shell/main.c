#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    printf("Current Work Directory: %s\n", getcwd());
    if (chdir("/home/calong") < 0) {
        printf("chdir failed!");
        return -1;
    }
    printf("Current Work Directory: %s\n", getcwd());
    return 0;
}