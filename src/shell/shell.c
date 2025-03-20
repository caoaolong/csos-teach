#include <csos/syscall.h>
#include <csos/shell.h>
#include <csos/string.h>
#include <csos/term.h>
#include <fs/file.h>

static int cmd_exec_help(int argc, char **argv);
static int cmd_exec_clear(int argc, char **argv);

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
    }
};

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
    char *cmd = shell->cmd;
    for (shell_cmd_t *pc = shell->cmd_begin; pc != shell->cmd_end; pc++) {
        if (!strcmp(cmd, pc->name)) {
            pc->cmd_exec(0, NULL);
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

static int cmd_exec_help(int argc, char **argv)
{
    printf("Hello,World!\n");
}

static int cmd_exec_clear(int argc, char **argv)
{
    clear();
}