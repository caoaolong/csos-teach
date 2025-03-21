#include <csos/syscall.h>
#include <csos/shell.h>
#include <csos/string.h>
#include <csos/term.h>
#include <fs/file.h>

static int cmd_exec_help(struct shell_t *shell);
static int cmd_exec_clear(struct shell_t *shell);
static int cmd_exec_echo(struct shell_t *shell);

static shell_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help\tshow all supported commands",
        .cmd_exec = cmd_exec_help
    },
    {
        .name = "clear",
        .usage = "clear\tclear the current screen",
        .cmd_exec = cmd_exec_clear
    },
    {
        .name = "echo",
        .usage = "echo\techo the string to screen\n"
                 "    \t[-n <times>] <string>",
        .cmd_exec = cmd_exec_echo
    }
};

#define is_digit(c)    ((c) >= '0' && (c) <= '9')
static int atoi(char *s)
{
    int i = 0;
    while (is_digit(*s))
        i = i * 10 + *(s++) - '0';
    return i;
}

static char *shell_get_arg(shell_t *shell)
{
    int pread = shell->pcread;
    while (*(shell->cmd + shell->pcread) != ' ' && *(shell->cmd + shell->pcread) != 0) {
        shell->pcread++;
    }
    if (shell->cmd[shell->pcread] == ' ') {
        shell->cmd[shell->pcread++] = 0;
    }
    return shell->cmd + pread;
}

void shell_init(shell_t *shell)
{
    shell->cmd_begin = cmd_list;
    shell->cmd_end = cmd_list + sizeof(cmd_list) / sizeof(shell_cmd_t);
    shell->prompt = CMD_PROMPT;
    memset(shell->cmd, 0, CMD_MAX_SIZE);
    shell->pcread = shell->pcwrite = 0;
}

void shell_prompt(shell_t *shell)
{
    printf("%s", shell->prompt);
}

void shell_exec(shell_t *shell)
{
    BOOL found = FALSE;
    char *cmd = shell_get_arg(shell);
    for (shell_cmd_t *pc = shell->cmd_begin; pc != shell->cmd_end; pc++) {
        if (!strcmp(cmd, pc->name)) {
            pc->cmd_exec(shell);
            found = TRUE;
            break;
        }
    }
    memset(shell->cmd, 0, CMD_MAX_SIZE);
    shell->pcread = shell->pcwrite = 0;
    if (!found) {
        printf("command not found\n");
    }
}

void shell_putc(shell_t *shell, char ch)
{
    if (ch == '\b') {
        shell->cmd[shell->pcwrite] = 0;
    } else {
        shell->cmd[shell->pcwrite++] = ch;
    }
}

static int cmd_exec_help(struct shell_t *shell)
{
    for (shell_cmd_t *pc = shell->cmd_begin; pc != shell->cmd_end; pc++)
        printf("%s\n", pc->usage);
}

static int cmd_exec_clear(struct shell_t *shell)
{
    clear();
}

static int cmd_exec_echo(struct shell_t *shell)
{
    // [-n] or <string>
    char *arg = shell_get_arg(shell);
    int times = 1;
    if (!strcmp(arg, "-n")) {
        // <times>
        arg = shell_get_arg(shell);
        times = atoi(arg);
        // <string>
        arg = shell_get_arg(shell);
    }
    for (int i = 0; i < times; i++)
        printf("\033[32;40m%s\033[0m\n", arg);
}