#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stddef.h>

#define MAX_PROCESSES 150
#define MAX_ARGUMENTS 10

int parse_command_strings(int argc,
                          char *argv[],
                          char ***out_cmds,
                          int *out_count);

void free_command_strings(char **cmds, int count);

#endif
