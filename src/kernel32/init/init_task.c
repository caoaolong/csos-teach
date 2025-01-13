#include <csos/syscall.h>

void init_entry()
{
    int pid = fork();
    if (pid < 0) {
        logf("fork failed...", 0);
    } else if (pid == 0) {
        logf("child task...", 0);
        char *argv[] = {"arg1", "arg2"};
        execve("/shell.elf", argv, NULL);
    } else {
        logf("parent: child task id = %d", pid);
    }
    uint32_t counter = 0;
    while (TRUE)
    {
        logf("task id = %d", getpid());
        logf("  counter = %d", counter++);
        yield();
        if (pid == 0 && counter >= 100) {
            exit(0);
        }
    }
}