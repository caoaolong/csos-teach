#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    // wd
    printf("Current Work Directory: %s\n", getcwd());
    if (chdir("/home/calong") < 0) {
        printf("chdir failed!");
        return -1;
    }
    printf("Current Work Directory: %s\n", getcwd());
    
    // file
    FILE *file = fopen("/etc/bashrc", "r");
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

    // dir
    printf("\n");
    DIR *dir = opendir("/home/calong");
    if (dir == NULL) return -1;
    struct dirent *dirent;
    while ((dirent = readdir(dir)) != NULL) {
        printf("%c %10s %6d\n",
            dirent->d_type == FT_DIR ? 'd' : 'f',
            dirent->d_name, 
            dirent->d_reclen);
    }
    closedir(dir);
    return 0;
}