#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <sys/types.h>

#define MAX_PROCESSES 150
#define MAX_ARGUMENTS 10

typedef struct process {
    pid_t pid;
    char **argv;
    int argc; 
    int is_stopped;
} process_t;

#endif
