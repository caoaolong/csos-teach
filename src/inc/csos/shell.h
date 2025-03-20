#ifndef CSOS_SHELL_H
#define CSOS_SHELL_H

#include <types.h>

#define CMD_MAX_SIZE    128
#define CMD_PROMPT      "csos >>"

typedef struct shell_cmd_t {
    const char *name;
    const char *usage;
    int (*cmd_exec)(int argc, char **argv);
} shell_cmd_t;

typedef struct shell_t {
    char cmd[CMD_MAX_SIZE];
    int pcread, pcwrite;

    shell_cmd_t *cmd_begin;
    shell_cmd_t *cmd_end;
    const char *prompt;
} shell_t;

void shell_init(shell_t *shell);

void shell_prompt(shell_t *shell);

void shell_exec(shell_t *shell);

void shell_putc(shell_t *shell, char ch);

#endif