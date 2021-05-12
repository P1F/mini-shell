#ifndef PARSE_H
#define PARSE_H

#include "defs.h"

char *trim_whitespace(char *str);
void error(char *message);
void parse_pipes(char command[], int cmd_len, Commands *commands);
void parse_command(char command[], Process *process);
char *parse_redirect(char command[], Process *process);
void parse_args(char command[], Process *process);

#endif