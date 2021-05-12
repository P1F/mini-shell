#ifndef DEFS_H
#define DEFS_H

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
    char cmd[MAX_CMD_LEN];
    char *argv[MAX_ARGS + 1];
    char *input;
    char *output;
    Process *pipe_in;
    Process *pipe_out;
};

struct Commands
{
    char **cmds;
    int cmd_cnt;
};

#endif