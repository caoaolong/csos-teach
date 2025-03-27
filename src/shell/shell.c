#include <csos/syscall.h>
#include <csos/shell.h>
#include <csos/string.h>
#include <csos/term.h>
#include <fs/file.h>
#include <fs/dir.h>

static int cmd_exec_help(struct shell_t *shell);
static int cmd_exec_clear(struct shell_t *shell);
static int cmd_exec_echo(struct shell_t *shell);
static int cmd_exec_pwd(struct shell_t *shell);
static int cmd_exec_ls(struct shell_t *shell);
static int cmd_exec_cd(struct shell_t *shell);
static int cmd_exec_mkdir(struct shell_t *shell);
static int cmd_exec_rmdir(struct shell_t *shell);

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
    },
    {
        .name = "pwd",
        .usage = "pwd\tprint the current workspace directory\n",
        .cmd_exec = cmd_exec_pwd
    },
    {
        .name = "ls",
        .usage = "ls\tlist the files of path\n"
                 "  \t<path>",
        .cmd_exec = cmd_exec_ls
    },
    {
        .name = "cd",
        .usage = "cd\tchange the current directory\n"
                 "  \t<path>",
        .cmd_exec = cmd_exec_cd
    },
    {
        .name = "mkdir",
        .usage = "mkdir\tmake directory\n"
                 "     \t<path>",
        .cmd_exec = cmd_exec_mkdir
    },
    {
        .name = "rmdir",
        .usage = "rmdir\tremove directory\n"
                 "     \t<path>",
        .cmd_exec = cmd_exec_rmdir
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
    memset(shell->cmd, 0, CMD_MAX_SIZE);
    strcpy(shell->cwd, getcwd());
    shell->pcread = shell->pcwrite = 0;
}

void shell_prompt(shell_t *shell)
{
    printf("[%s:%s]$", "root", shell->cwd);
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
    return 0;
}

static int cmd_exec_clear(struct shell_t *shell)
{
    clear();
    return 0;
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

static int cmd_exec_pwd(struct shell_t *shell)
{
    printf("%s\n", getcwd());
    return 0;
}

static int cmd_exec_ls(struct shell_t *shell)
{
    char *arg = shell_get_arg(shell);
    if (*arg == 0) {
        arg = shell->cwd;
    }
    DIR *dir = opendir(arg);
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

static int cmd_exec_cd(struct shell_t *shell)
{
    char *path = shell_get_arg(shell);
    int err = chdir(path);
    if (err == 0) {
        strcpy(shell->cwd, getcwd());
    }
    return err;
}

static int cmd_exec_mkdir(struct shell_t *shell)
{
    char *path = shell_get_arg(shell);
    if (*path == 0) 
        return 0;
    return mkdir(path);
}

static int cmd_exec_rmdir(struct shell_t *shell)
{
    char *path = shell_get_arg(shell);
    if (*path == 0) 
        return 0;
    if (!strcmp(shell->cwd, path))
        return 0;
    return rmdir(path);
}