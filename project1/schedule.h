#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <sys/types.h>

#define MAX_PROCESSES 150
#define MAX_ARGUMENTS 10

typedef struct process {
    pid_t pid;
    char **argv;    /* NULL-terminated argv for execvp */
    int argc;       /* number of entries in argv excluding NULL */
    int is_stopped; /* 1 if currently stopped, 0 if running */
} process_t;

#endif
