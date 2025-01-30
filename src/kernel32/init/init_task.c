#include <csos/syscall.h>

void init_entry()
{
    char *argv[] = {"arg1", "arg2", NULL};
    execve("/shell.elf", argv, NULL);
}