#include "minishell.h"

int abortar = 0;

int main()
{
    int cmd_len;
    char buf[MAX_FULL_CMD_LEN];
    Commands commands;
    Process *processes;

    write(STDOUT_FILENO, "Welcome to my Shell!\n", 21);

    while (1)
    {
        write(STDOUT_FILENO, "cmd > ", 6);

        cmd_len = read(STDIN_FILENO, buf, MAX_FULL_CMD_LEN);
        if (cmd_len > 1)
        {
            commands.cmd_cnt = 0;

            buf[cmd_len - 1] = '\0';
            parse_pipes(buf, cmd_len, &commands);
            processes = create_processes(commands);

            if (abortar == 1)
                write(STDOUT_FILENO, "Limite de argumentos excedido!\n", 31);
            else
            {
                for (int i = 0; i < commands.cmd_cnt; i++)
                {
                    printf("process[%d].command = '%s'\n", i, processes[i].command);
                    for (int j = 0; j < MAX_ARGS; j++)
                        printf("process[%d].arg[%d] = '%s'\n", i, j, processes[i].args[j]);
                    printf("process[%d].input_file = '%s'\n", i, processes[i].input_file);
                    printf("process[%d].output_file = '%s'\n\n", i, processes[i].output_file);
                }
                printf("\n\n");
            }

            free(processes);
            free(commands.cmds);
        }
    }

    return 0;
}

char *trim_whitespace(char *str)
{
    char *end;

    // Trim leading space
    while ((unsigned char)*str == ' ')
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (unsigned char)*end == ' ')
        end--;

    // Write new null terminator character
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
    process->output_file = output_file;

    token = strtok_r(token, "<", &input_file);
    token = trim_whitespace(token);
    if (strcmp(input_file, "") == 0)
        input_file = NULL;
    else
        input_file = trim_whitespace(input_file);
    process->input_file = input_file;

    return token;
}

void parse_args(char command[], Process *process)
{
    char *token;
    int index = 0;

    for (int i = 0; i < MAX_ARGS; i++)
        process->args[i] = NULL;

    token = strtok(command, " ");
    strcpy(process->command, token);
    while (token != NULL)
    {
        if (index >= 11)
        {
            abortar = 1;
            break;
        }
        token = strtok(NULL, " ");
        process->args[index] = token;
        index++;
    }
}

void parse_command(char command[], Process *process)
{
    command = parse_redirect(command, process);
    parse_args(command, process);
}

Process *create_processes(Commands commands)
{
    Process *processes;
    processes = malloc(commands.cmd_cnt * sizeof(Process));
    for (int i = 0; i < commands.cmd_cnt; i++)
    {
        parse_command(commands.cmds[i], &processes[i]);
        if (i == 0)
        { // ultimo processo filho
            processes[i].pipe_in = NULL;
            processes[i].pipe_out = &processes[i + 1];
        }
        else if (i == commands.cmd_cnt - 1)
        { // processo raiz
            processes[i].pipe_in = &processes[i - 1];
            processes[i].pipe_out = NULL;
        }
        else
        {
            processes[i].pipe_in = &processes[i - 1];
            processes[i].pipe_out = &processes[i + 1];
        }
    }

    return processes;
}