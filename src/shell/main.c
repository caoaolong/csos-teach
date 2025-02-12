#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    FILE *file = fopen("/tmp.txt", "r");
    char buf[100];
    int size = fgets(file, buf, sizeof(buf));
    if (size < 0)
        return -1;
    fclose(file);
    return 0;
}