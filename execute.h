#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"

void start_shell();
Process *create_processes(Commands commands);
void run_processes(Process *processes, int cmd_cnt);
void execute_command(Process *process);
void execute(Process *process);

#endif