#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    FILE *file = fopen("kernel.h", "r");
    int size = 0;
    char buf[100];
    while (size < file->size) {
        int nbytes = fgets(file, buf, sizeof(buf));
        if (nbytes < 0) {
            printf("read failed\n");
            break;
        }
        printf("%s", buf);
        size += nbytes;
    }
    fclose(file);
    return 0;
}