#ifndef CSOS_SHELL_H
#define CSOS_SHELL_H

#include <list.h>
#include <csos/term.h>

#define CMD_MAX_ARGS    8
#define CMD_MAX_SIZE    64

struct shell_t;

typedef struct shell_history_t {
    char cmd[CMD_MAX_SIZE];
    list_node_t node;
} shell_history_t;

typedef struct shell_cmd_t {
    const char *name;
    const char *usage;
    int (*cmd_exec)(struct shell_t *shell);
} shell_cmd_t;

typedef struct shell_t {
    char cmd[CMD_MAX_SIZE];
    char cwd[CMD_MAX_SIZE];
    int pcread, pcwrite;
    shell_cmd_t *cmd_begin;
    shell_cmd_t *cmd_end;
    // 历史命令
    uint32_t now;
    list_t *history;
    // 控制台属性
    term_t term;
} shell_t;

void shell_init(shell_t *shell);

void shell_prompt(shell_t *shell);

void shell_exec(shell_t *shell);

void shell_putc(shell_t *shell, char ch);

void shell_cmd_up(shell_t *shell);

void shell_cmd_down(shell_t *shell);

#endif