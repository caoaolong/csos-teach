#include <csos/syscall.h>
#include <fs.h>

int main(int argc, char *argv[])
{
    DIR *dir = opendir("/tmp");
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