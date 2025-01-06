#include <csos/syscall.h>

void init_entry()
{
    int pid = fork();
    if (pid < 0) {
        logf("fork failed...", 0);
    } else if (pid == 0) {
        logf("child task...", 0);
    } else {
        logf("parent: child task id = %d", pid);
    }

    while (TRUE)
    {
        logf("task id = %d", getpid());
        sleep(1000);
    }
}