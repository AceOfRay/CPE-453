#define _POSIX_C_SOURCE 200809L

#include "command_parser.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>



void free_command_strings(char **cmds, int count)
{
    for (int i = 0; i < count; i++) {
        free(cmds[i]);
    }
    free(cmds);
}

static int append_command(char ***cmds,
                          int *count,
                          char *const tokens[],
                          int ntok)
{
    size_t len = 0;
    int prefix_exec = 0;
    if (ntok > 0 && strchr(tokens[0], '/') == NULL) {
        struct stat st;
        if (stat(tokens[0], &st) == 0 && S_ISREG(st.st_mode) &&
            access(tokens[0], X_OK) == 0) {
            prefix_exec = 1;
        }
    }
    for (int i = 0; i < ntok; i++) {
        if (i == 0 && prefix_exec) {
            len += 2;
        }
        len += strlen(tokens[i]);
        if (i + 1 < ntok) {
            len += 1;
        }
    }

    char *cmd = (char *)malloc(len + 1);

    size_t off = 0;
    for (int i = 0; i < ntok; i++) {
        size_t tlen = strlen(tokens[i]);
        if (i == 0 && prefix_exec) {
            cmd[off++] = '.';
            cmd[off++] = '/';
        }
        memcpy(cmd + off, tokens[i], tlen);
        off += tlen;
        if (i + 1 < ntok) {
            cmd[off++] = ' ';
        }
    }
    cmd[off] = '\0';

    (*cmds)[*count] = cmd;
    (*count)++;
    return 0;
}

int parse_command_strings(int argc,
                          char *argv[],
                          char ***out_cmds,
                          int *out_count)
{
    *out_cmds = NULL;
    *out_count = 0;

    char *end = NULL;
    strtol(argv[1], &end, 10);

    char **cmds = (char **)calloc(MAX_PROCESSES, sizeof(*cmds));

    int count = 0;
    int i = 2;
    while (i < argc) {
        char *tokens[MAX_ARGUMENTS + 1];
        int ntok = 0;
        while (i < argc && strcmp(argv[i], ":") != 0) {
            tokens[ntok++] = argv[i++];
        }

        append_command(&cmds, &count, tokens, ntok);

        if (i < argc && strcmp(argv[i], ":") == 0) {
            i++;
        }
    }

    *out_cmds = cmds;
    *out_count = count;
    return 0;
}
