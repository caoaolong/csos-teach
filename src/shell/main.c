#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    FILE *file = fopen("/root/bashrc", "w");
    char content[] = "Test Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write File"
        "Test Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write File"
        "Test Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write File"
        "Test Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write File"
        "Test Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write File"
        "Test Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write FileTest Write File";
    fputs(file, content, sizeof(content));
    fclose(file);
    return 0;
}