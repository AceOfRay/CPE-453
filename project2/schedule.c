#define _POSIX_C_SOURCE 200809L

#include "schedule.h"
#include "queue.h"

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>



queue_t *spawn_necessary_processes(char **cmds, int count)
{
    queue_t *queue = queue_create();
    if (!queue) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            raise(SIGSTOP);
            execlp("sh", "sh", "-c", cmds[i], (char *)NULL);
            _exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            queue_push(queue, (void *)(intptr_t)pid);
        }
    }

    return queue;
}

int main(int argc, char *argv[])
{
    char **cmds = NULL;
    int count = 0;

    if (parse_command_strings(argc, argv, &cmds, &count) != 0) {
        return EXIT_FAILURE;
    }

    long quantum_ms = strtol(argv[1], NULL, 10);

    unsigned int quantum_seconds = (unsigned int)(quantum_ms / 1000);
    if (quantum_seconds == 0) {
        quantum_seconds = 1;
    }

    // spawn the necessary processes in a stopped state
    queue_t *queue = spawn_necessary_processes(cmds, count);

    // while the queue is active, loop the children for quantum time length.
    while (queue && !queue_is_empty(queue)) {

        pid_t pid = (pid_t)(intptr_t)queue_pop(queue);
        kill(pid, SIGCONT);
        sleep(quantum_seconds);

        int status = 0;
        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == 0) {
            kill(pid, SIGSTOP);
            queue_push(queue, (void *)(intptr_t)pid);
        }
    }
    queue_destroy(&queue, NULL);

    free_command_strings(cmds, count);
    return EXIT_SUCCESS;
}
