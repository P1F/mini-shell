#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_FULL_CMD_LEN 500
#define COL 4
#define MAX_CMD_LEN 50
#define MAX_ARGS 10

typedef struct Process Process;
typedef struct Commands Commands;

struct Process
{
    char command[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    char *input_file;
    char *output_file;
    Process *pipe_in;
    Process *pipe_out;
};

struct Commands
{
    char **cmds;
    int cmd_cnt;
};

void parse_pipes(char command[], int cmd_len, Commands *commands);
void parse_command(char command[], Process *process);
char *parse_redirect(char command[], Process *process);
void parse_args(char command[], Process *process);
Process *create_processes(Commands commands);
char *trim_whitespace(char *str);

#endif