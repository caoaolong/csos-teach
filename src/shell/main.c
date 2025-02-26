#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    remove("/home/calong/banner.txt");
    return 0;
}