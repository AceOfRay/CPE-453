#define _POSIX_C_SOURCE 200809L

#include "command_parser.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s quantum prog1 [args] : prog2 [args] : ...\n"
            "  quantum is in milliseconds\n",
            prog);
}

void free_command_strings(char **cmds, int count)
{
    if (!cmds) {
        return;
    }
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
    int prefix_exec = (ntok > 0 && strchr(tokens[0], '/') == NULL);
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
    if (!cmd) {
        return -1;
    }

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
    if (!out_cmds || !out_count) {
        fprintf(stderr, "Internal error: invalid output parameters\n");
        return -1;
    }
    *out_cmds = NULL;
    *out_count = 0;

    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    char *end = NULL;
    errno = 0;
    long quantum_ms = strtol(argv[1], &end, 10);
    if (errno != 0 || !end || *end != '\0' || quantum_ms <= 0) {
        fprintf(stderr, "Invalid quantum: %s\n", argv[1]);
        return -1;
    }

    char **cmds = (char **)calloc(MAX_PROCESSES, sizeof(*cmds));
    if (!cmds) {
        fprintf(stderr, "Out of memory\n");
        return -1;
    }

    int count = 0;
    int i = 2;
    while (i < argc) {
        if (strcmp(argv[i], ":") == 0) {
            fprintf(stderr, "Empty command before ':'\n");
            free_command_strings(cmds, count);
            return -1;
        }

        char *tokens[MAX_ARGUMENTS + 1];
        int ntok = 0;
        while (i < argc && strcmp(argv[i], ":") != 0) {
            if (ntok >= (MAX_ARGUMENTS + 1)) {
                fprintf(stderr,
                        "Too many arguments for one process (max %d)\n",
                        MAX_ARGUMENTS);
                free_command_strings(cmds, count);
                return -1;
            }
            tokens[ntok++] = argv[i++];
        }

        if (ntok == 0) {
            fprintf(stderr, "Empty command\n");
            free_command_strings(cmds, count);
            return -1;
        }
        if (count >= MAX_PROCESSES) {
            fprintf(stderr, "Too many processes (max %d)\n", MAX_PROCESSES);
            free_command_strings(cmds, count);
            return -1;
        }

        if (append_command(&cmds, &count, tokens, ntok) != 0) {
            fprintf(stderr, "Out of memory\n");
            free_command_strings(cmds, count);
            return -1;
        }

        if (i < argc && strcmp(argv[i], ":") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Trailing ':' with no following command\n");
                free_command_strings(cmds, count);
                return -1;
            }
        }
    }

    if (count == 0) {
        fprintf(stderr, "No processes specified\n");
        free_command_strings(cmds, count);
        return -1;
    }

    *out_cmds = cmds;
    *out_count = count;
    return 0;
}
