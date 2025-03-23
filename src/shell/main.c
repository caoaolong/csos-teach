#include <csos/syscall.h>
#include <csos/shell.h>
#include <fs.h>

static shell_t shell;

int main(int argc, char *argv[])
{
    // stdio
    fopen(argv[0], "w");        // stdin
    dup(0);                     // stdout
    dup(0);                     // stderr

    // banner
    printf("\t\t  \033[33;40m   ___     ___     ___     ___   \033[0m\n");
    printf("\t\t  \033[34;40m  / __|   / __|   / _ \\   / __|  \033[0m\n");
    printf("\t\t  \033[35;40m | (__    \\__ \\  | (_) |  \\__ \\  \033[0m\n");
    printf("\t\t  \033[36;40m  \\___|   |___/   \\___/   |___/  \033[0m\n\n");
    printf("\t\t  \033[37;40m CSOS %s (Kernel: %s)\033[0m  %s\n\n", OP_SYS_VERSION, KERNEL_VERSION, argv[0]);

    // shell
    shell_init(&shell);
    shell_prompt(&shell);
    while(TRUE) {
        char ch = getc();
        // 判断回车
        if (ch == '\n') {
            shell_exec(&shell);
            shell_prompt(&shell);
        } else {
            shell_putc(&shell, ch);
        }
    }
    return 0;
}