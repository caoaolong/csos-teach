#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    FILE *file = fopen("/kernel.h", "w");
    int size = lseek(file, 0, SEEK_END);
    printf("File size: %dKB\n", size / 1024);
    fputs(file, "Hello,World!", 12);
    printf("File size: %dKB\n", file->size / 1024);
    fclose(file);
    return 0;
}