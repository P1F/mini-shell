#include "parse.h"

void error(char *message)
{
    perror(message);
    _exit(1);
}

char *trim_whitespace(char *str)
{
    char *end;

    while ((unsigned char)*str == ' ')
        str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && (unsigned char)*end == ' ')
        end--;

    end[1] = '\0';

    return str;
}

void parse_pipes(char command[], int cmd_len, Commands *commands)
{
    char *token;

    for (int i = 0; i < cmd_len; i++)
    {
        if (command[i] == '|')
            commands->cmd_cnt += 1;
    }
    commands->cmd_cnt += 1;

    commands->cmds = malloc(commands->cmd_cnt * sizeof(char *));
    for (int i = 0; i < commands->cmd_cnt; i++)
    {
        commands->cmds[i] = malloc(MAX_CMD_LEN * sizeof(char));
        if (i == 0)
            token = strtok(command, "|");
        else
            token = strtok(NULL, "|");

        token = trim_whitespace(token);
        strcpy(commands->cmds[i], token);
    }
}

char *parse_redirect(char command[], Process *process)
{
    char *token;
    char *output_file;
    char *input_file;

    /* CASOS PARA PREVER
    *   1) quando > vem antes de <
    *   2) quando tem mais de uma saída (>)
    */

    token = strtok_r(command, ">", &output_file);
    token = trim_whitespace(token);
    if (strcmp(output_file, "") == 0)
        output_file = NULL;
    else
        output_file = trim_whitespace(output_file);
    process->output = output_file;

    token = strtok_r(token, "<", &input_file);
    token = trim_whitespace(token);
    if (strcmp(input_file, "") == 0)
        input_file = NULL;
    else
        input_file = trim_whitespace(input_file);
    process->input = input_file;

    return token;
}

void parse_args(char command[], Process *process)
{
    char *token;
    int index = 0;

    for (int i = 0; i < MAX_ARGS; i++)
        process->argv[i] = NULL;

    token = strtok(command, " ");
    strcpy(process->cmd, token);
    process->argv[index] = token;
    while (token != NULL)
    {
        index++;
        if (index > MAX_ARGS + 1)
            error("Numero maximo de argumentos excedido!");
        token = strtok(NULL, " ");
        process->argv[index] = token;
    }
}

void parse_command(char command[], Process *process)
{
    command = parse_redirect(command, process);
    parse_args(command, process);
}