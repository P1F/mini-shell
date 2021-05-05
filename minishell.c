#include "minishell.h"

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

            run_processes(processes, commands.cmd_cnt);

            free(processes);
            free(commands.cmds);
        }
    }

    return 0;
}

void error(char *message)
{
    perror(message);
    _exit(1);
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
    process->args[index] = token;
    while (token != NULL)
    {
        index++;
        if (index > MAX_ARGS + 1)
            error("Numero maximo de argumentos excedido!");
        token = strtok(NULL, " ");
        process->args[index] = token;
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
            if (commands.cmd_cnt != 1)
                processes[i].pipe_out = &processes[i + 1];
            else
                processes[i].pipe_out = NULL;
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

void execute(Process *process)
{
    int fd;

    if (process->input_file != NULL)
    {
        fd = open(process->input_file, O_RDONLY);
        dup2(fd, STDIN_FILENO);
    }
    if (process->output_file != NULL)
    {
        fd = open(process->output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        dup2(fd, STDOUT_FILENO);
    }
    close(fd);

    execve(process->command, process->args, NULL);
}

void execute_command(Process *process)
{
    if (process->pipe_in == NULL && process->pipe_out == NULL)
        execute(process);
    else
    {
        int pipefd[2];
        pipe(pipefd);

        pid_t child_pid = fork();
        if (child_pid == 0)
        { // filho
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            if (process->pipe_in != NULL)
                execute_command(process->pipe_in);
            execute(process);
        }
        else
        { // pai
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            wait(NULL);
            if (process->pipe_out == NULL)
                execute(process);
        }
    }
}

void run_processes(Process *processes, int cmd_cnt)
{
    // int pipefd[2];
    // pipe(pipefd);

    pid_t child_pid = fork();
    if (child_pid == 0)
    { // filho
        // close(pipefd[1]);
        // dup2(pipefd[0], STDIN_FILENO);
        execute_command(&processes[cmd_cnt - 1]);
    }
    else
    { // pai
        // close(pipefd[0]);
        // dup2(pipefd[1], STDOUT_FILENO);
        wait(NULL);
    }
}