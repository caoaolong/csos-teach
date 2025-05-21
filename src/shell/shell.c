#include <csos/syscall.h>
#include <csos/shell.h>
#include <csos/string.h>
#include <csos/term.h>
#include <fs/file.h>
#include <fs/dir.h>
#include <netx/inet.h>

static int cmd_exec_help(struct shell_t *shell);
static int cmd_exec_clear(struct shell_t *shell);
static int cmd_exec_echo(struct shell_t *shell);
static int cmd_exec_pwd(struct shell_t *shell);
static int cmd_exec_ls(struct shell_t *shell);
static int cmd_exec_cd(struct shell_t *shell);
static int cmd_exec_mkdir(struct shell_t *shell);
static int cmd_exec_rmdir(struct shell_t *shell);
static int cmd_exec_exit(struct shell_t *shell);
static int cmd_exec_arp(struct shell_t *shell);
static int cmd_exec_ping(struct shell_t *shell);
static int cmd_exec_ifconf(struct shell_t *shell);
static int cmd_exec_netcat(struct shell_t *shell);
static int cmd_exec_lsof(struct shell_t *shell);
static int cmd_exec_test(struct shell_t *shell);

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
        .usage = "pwd\tprint the current workspace directory",
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
    },
    {
        .name = "exit",
        .usage = "exit\texit the current task",
        .cmd_exec = cmd_exec_exit
    },
    {
        .name = "arp",
        .usage = "arp\tmanage the arp map\n"
                 "   \t-a\tshow all arp map\n"
                 "   \t-c\tclean the arp map",
        .cmd_exec = cmd_exec_arp
    },
    {
        .name = "ping",
        .usage = "ping\tsend the ICMPv4 packet to target <ip>",
        .cmd_exec = cmd_exec_ping
    },
    {
        .name = "ifconfig",
        .usage = "ifconfig\tshow all netif device info",
        .cmd_exec = cmd_exec_ifconf
    },
    {
        .name = "nc",
        .usage = "nc\tnetcat tool to udp/tcp connect\n"
                 "  \t-l <port> listen to port for tcp\n"
                 "  \t-u -l <port> listen to port for udp\n"
                 "  \t<ip> <port> connect to tcp server",
        .cmd_exec = cmd_exec_netcat
    },
    {
        .name = "lsof",
        .usage = "lsof\tlist the open files of process\n"
                 "    \t-i list the ports of the process",
        .cmd_exec = cmd_exec_lsof
    },
    {
        .name = "test",
        .usage = "test\ttest the kernel function",
        .cmd_exec = cmd_exec_test
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

static shell_history_t *shell_new_history(const char *value)
{
    shell_history_t *cmd = (shell_history_t*)malloc(sizeof(shell_history_t));
    strcpy(cmd->cmd, value);
    list_node_init(&cmd->node);
    return cmd;
}

void shell_init(shell_t *shell)
{
    shell->cmd_begin = cmd_list;
    shell->cmd_end = cmd_list + sizeof(cmd_list) / sizeof(shell_cmd_t);
    memset(shell->cmd, 0, CMD_MAX_SIZE);
    strcpy(shell->cwd, getcwd());
    shell->pcread = shell->pcwrite = 0;

    shell->history = (list_t *)malloc(sizeof(list_t));
    list_init(shell->history);
    shell_history_t *ncmd = shell_new_history(shell->cmd);
    list_insert_back(shell->history, &ncmd->node);

    shell->now = (uint32_t)list_get_last(shell->history);
}

void shell_prompt(shell_t *shell)
{
    printf("[%s:%s]$", "root", shell->cwd);
    tcgetattr(stdin, &shell->term);
}

void shell_result(BOOL r, const char *string)
{
    if (r) {
        printf("[\033[32;40mO K\033[0m] %s\n", string);
    } else {
        printf("[\033[34;40mERR\033[0m] %s\n", string);
    }
}

static void shell_cmd_save(shell_t *shell)
{
    shell_history_t *ncmd = shell_new_history(shell->cmd);
    list_insert_node(shell->history, &ncmd->node, (list_node_t *)shell->now, 1);
}

void shell_exec(shell_t *shell)
{
    shell_cmd_save(shell);
    BOOL found = FALSE;
    char *cmd = shell_get_arg(shell);
    for (shell_cmd_t *pc = shell->cmd_begin; pc != shell->cmd_end; pc++) {
        if (!strcmp(cmd, pc->name)) {
            pc->cmd_exec(shell);
            found = TRUE;
            break;
        }
    }
    if (!found) {
        printf("command not found\n");
    }
    memset(shell->cmd, 0, CMD_MAX_SIZE);
    shell->pcread = shell->pcwrite = 0;
}

void shell_putc(shell_t *shell, char ch)
{
    if (ch == '\b' && shell->pcwrite > 0) {
        shell->cmd[--shell->pcwrite] = 0;
    } else if (ch == 0) {
        return;
    } else {
        shell->cmd[shell->pcwrite++] = ch;
    }
}

void shell_cmd_up(shell_t *shell)
{
    if (list_is_empty(shell->history))
        return;
    list_node_t *pnode = list_get_pre((list_node_t *)shell->now);
    shell_history_t *cmd = struct_from_field(pnode, shell_history_t, node);
    if (!cmd) 
        return;
    shell->now = (uint32_t)&cmd->node;
    // 替换当前命令
    tcsetattr(stdin, &shell->term);
    strcpy(shell->cmd, cmd->cmd);
    printf("\033[%d;%df%s\033[0f", shell->term.cr, shell->term.cc, cmd->cmd);
}

void shell_cmd_down(shell_t *shell)
{
    if (list_is_empty(shell->history))
        return;
    list_node_t *pnode = list_get_next((list_node_t *)shell->now);
    shell_history_t *cmd = struct_from_field(pnode, shell_history_t, node);
    if (!cmd) {
        tcsetattr(stdin, &shell->term);
        return;
    }
    shell->now = (uint32_t)&cmd->node;
    // 替换当前命令
    tcsetattr(stdin, &shell->term);
    printf("\033[%d;%df%s\033[0f", shell->term.cr, shell->term.cc, cmd->cmd);
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
    int err = rmdir(path);
    if (err == 0) {
        shell_result(TRUE, "mkdir ok");
    } else {
        shell_result(FALSE, "mkdir failed");
    }
    return 0;
}

static int cmd_exec_rmdir(struct shell_t *shell)
{
    char *path = shell_get_arg(shell);
    if (*path == 0) 
        return 0;
    if (!strcmp(shell->cwd, path)) {
        shell_result(FALSE, "can't remove the current diectory");
        return 0;
    }
    int err = rmdir(path);
    if (err == 0) {
        shell_result(TRUE, "rmdir ok");
    } else {
        shell_result(FALSE, "rmdir failed");
    }
    return 0;
}

static int cmd_exec_exit(struct shell_t *shell)
{
    int stats;
    if (wait(&stats) < 0) {
        shell_result(FALSE, "wait failed");
    } else {
        clear();
        shell_result(TRUE, "task exited");
        exit(stats);
    }
    return 0;
}

static int cmd_exec_arp(struct shell_t *shell)
{
    char *arg = shell_get_arg(shell);
    if (!strcmp(arg, "-c")) {
        arpc();
        shell_result(TRUE, "arp map cleaned");
        return 0;
    } else if (!strcmp(arg, "-a")) {
    arp_map_data_t arp_map;
    arpl(&arp_map);
    printf("%15s     %17s\n", "IP", "MAC");
    printf("---------------     -----------------\n");
    for (int i = 0; i < arp_map.idx; i++) {
        printf("%3d.%3d.%3d.%3d     %02x:%02x:%02x:%02x:%02x:%02x\n",
            arp_map.items[i].ip[0], arp_map.items[i].ip[1],
            arp_map.items[i].ip[2], arp_map.items[i].ip[3],
            arp_map.items[i].mac[0], arp_map.items[i].mac[1],
            arp_map.items[i].mac[2], arp_map.items[i].mac[3],
            arp_map.items[i].mac[4], arp_map.items[i].mac[5]);
    }
        return 0;
    } else {
        shell_result(FALSE, "invalid option");
        return -1;
    }
}

static int cmd_exec_ping(struct shell_t *shell)
{
    ping(shell_get_arg(shell));
}

static int cmd_exec_ifconf(struct shell_t *shell)
{
    netif_dev_t *devs = (netif_dev_t *)malloc(sizeof(netif_dev_t) * 4);
    int devc;
    ifconf(devs, &devc);
    for (int i = 0; i < devc; i++) {
        netif_dev_t *dev = &devs[i];
        printf("%s: Status: ", dev->name);
        if (dev->status == NETIF_STATUS_DOWN) {
            printf("\033[34;40mDown\033[0m\n");
        } else if (dev->status == NETIF_STATUS_REQUESTED) {
            printf("\033[36;40mLinking...\033[0m\n");
        } else if (dev->status == NETIF_STATUS_ACK) {
            printf("\033[32;40mUp\033[0m\n");
        }
        printf("    IPv4 Address: %d.%d.%d.%d, Subnet Mask: %d.%d.%d.%d\n", 
            dev->ipv4[0], dev->ipv4[1], dev->ipv4[2], dev->ipv4[3],
            dev->mask[0], dev->mask[1], dev->mask[2], dev->mask[3]);
        printf("    Gateway IPv4 Address: %d.%d.%d.%d\n", dev->gwv4[0], dev->gwv4[1], dev->gwv4[2], dev->gwv4[3]);
        printf("    Client Mac Address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
            dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);
    }
}

static int cmd_exec_netcat(struct shell_t *shell)
{
    return 0;
}

static int cmd_exec_lsof(struct shell_t *shell)
{
    char *arg = shell_get_arg(shell);
    port_t port;
    uint16_t cp = 0, np = 0;
    printf("%5s %5s %10s\n", "PID", "PORT", "PTYPE");
    printf("----- ----- ----------\n");
    if (!strcmp(arg, "-i")) {
        do {
            enum_port(&port, &cp, &np);
            if (port.status == PORT_DOWN) {
                continue;
            }
            printf("%5d %5d ", port.pid, cp);
            if (port.ptype == DBT_TCP) {
                if (port.status == PORT_UP) {
                    printf("\033[32;40m%10s\033[0m\n", "TCP/UP");
                } else if (port.status == PORT_BUSY) {
                    printf("\033[36;40m%10s\033[0m\n", "TCP/BUSY");
                }
            } else if (port.ptype == DBT_UDP) {
                if (port.status == PORT_UP) {
                    printf("\033[32;40m%10s\033[0m\n", "UDP/UP");
                } else if (port.status == PORT_BUSY) {
                    printf("\033[36;40m%10s\033[0m\n", "UDP/BUSY");
                }
            }
            cp = np;
        } while (np > 0);
    }

    return 0;
}

static int cmd_exec_test(struct shell_t *shell)
{
    test();
    return 0;
}