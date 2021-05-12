
#include "execute.h"
#include "parse.c"

void start_shell()
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

    if (process->input != NULL)
    {
        fd = open(process->input, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (process->output != NULL)
    {
        fd = open(process->output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execve(process->cmd, process->argv, NULL);
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

            if (process->pipe_out == NULL)
                execute(process);

            wait(NULL);
        }
    }
}

void run_processes(Process *processes, int cmd_cnt)
{
    pid_t child_pid = fork();
    if (child_pid == 0)
    { // filho
        execute_command(&processes[cmd_cnt - 1]);
    }
    else
    { // pai
        wait(NULL);
    }
}