#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    mkdir("/root/packages");
    return 0;
}