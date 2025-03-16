#include <csos/syscall.h>

void init_entry()
{
    char *argv[] = {"/dev/tty?", NULL};
    for (int i = 0; i < TTY_DEV_NR; i++)
    {
        int pid = fork();
        if (pid < 0) {
            break;
        } else if (pid == 0) {
            argv[0][8] = '0' + i;
            execve("/shell.elf", argv, NULL);
        }
    }
    while(TRUE) sleep(1000);
}