#define _POSIX_C_SOURCE 200809L

#include "schedule.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t g_got_sigalrm = 0;
volatile sig_atomic_t g_got_sigchld = 0;

void on_sigalrm(int signo)
{
    (void)signo;
    g_got_sigalrm = 1;
}

void on_sigchld(int signo)
{
    (void)signo;
    g_got_sigchld = 1;
}

void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s quantum prog1 [args] : prog2 [args] : ...\n"
            "  quantum is in milliseconds\n",
            prog);
}

void die_perror(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void *xcalloc(size_t n, size_t sz)
{
    void *p = calloc(n, sz);
    if (!p) {
        fprintf(stderr, "calloc failed\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

char *xstrdup(const char *s)
{
    char *d = strdup(s);
    if (!d) {
        fprintf(stderr, "strdup failed\n");
        exit(EXIT_FAILURE);
    }
    return d;
}

process_t *process_create(char *const tokens[], int argc)
{
    process_t *p = xcalloc(1, sizeof(*p));
    p->argc = argc;
    p->pid = -1;
    p->is_stopped = 0;

    p->argv = xcalloc((size_t)argc + 1, sizeof(char *));
    for (int i = 0; i < argc; i++) {
        p->argv[i] = xstrdup(tokens[i]);
    }
    p->argv[argc] = NULL;

    return p;
}

void process_destroy(process_t **pp)
{
    if (!pp || !*pp) {
        return;
    }
    process_t *p = *pp;
    if (p->argv) {
        for (int i = 0; i < p->argc; i++) {
            free(p->argv[i]);
        }
        free(p->argv);
    }
    free(p);
    *pp = NULL;
}

int find_index_by_pid(process_t *const procs[], int n, pid_t pid)
{
    for (int i = 0; i < n; i++) {
        if (procs[i] && procs[i]->pid == pid) {
            return i;
        }
    }
    return -1;
}

void handle_child_status(process_t *procs[], int n, int *live_count, pid_t pid, int st)
{
    int idx = find_index_by_pid((process_t *const *)procs, n, pid);
    if (idx < 0) {
        return;
    }

    if (WIFSTOPPED(st)) {
        procs[idx]->is_stopped = 1;
        return;
    }

    if (WIFEXITED(st) || WIFSIGNALED(st)) {
        process_destroy(&procs[idx]);
        if (live_count && *live_count > 0) {
            (*live_count)--;
        }
    }
}

void reap_any_children(process_t *procs[], int n, int *live_count)
{
    for (;;) {
        int st;
        pid_t pid = waitpid(-1, &st, WNOHANG | WUNTRACED);
        if (pid == 0) {
            return;
        }
        if (pid < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == ECHILD) {
                return;
            }
            die_perror("waitpid");
        }
        handle_child_status(procs, n, live_count, pid, st);
    }
}

int next_live_index(process_t *const procs[], int n, int start)
{
    if (n <= 0) {
        return -1;
    }
    int i = start;
    for (int step = 0; step < n; step++) {
        i = (i + 1) % n;
        if (procs[i]) {
            return i;
        }
    }
    return -1;
}

void set_quantum_timer(long quantum_ms)
{
    struct itimerval it;
    memset(&it, 0, sizeof(it));

    it.it_value.tv_sec = quantum_ms / 1000;
    it.it_value.tv_usec = (quantum_ms % 1000) * 1000;

    if (setitimer(ITIMER_REAL, &it, NULL) < 0) {
        die_perror("setitimer");
    }
}

void cancel_timer(void)
{
    struct itimerval it;
    memset(&it, 0, sizeof(it));
    (void)setitimer(ITIMER_REAL, &it, NULL);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *end = NULL;
    errno = 0;
    long quantum_ms = strtol(argv[1], &end, 10);
    if (errno != 0 || !end || *end != '\0' || quantum_ms <= 0) {
        fprintf(stderr, "Invalid quantum: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    process_t **procs = xcalloc(MAX_PROCESSES, sizeof(*procs));
    int nprocs = 0;

    int i = 2;
    while (i < argc) {
        if (strcmp(argv[i], ":") == 0) {
            fprintf(stderr, "Empty command before ':'\n");
            return EXIT_FAILURE;
        }

        char *tokens[MAX_ARGUMENTS + 1]; /* program + up to MAX_ARGUMENTS args */
        int ntok = 0;
        while (i < argc && strcmp(argv[i], ":") != 0) {
            if (ntok >= (int)(sizeof(tokens) / sizeof(tokens[0]))) {
                fprintf(stderr,
                        "Too many arguments for one process (max %d)\n",
                        MAX_ARGUMENTS);
                return EXIT_FAILURE;
            }
            tokens[ntok++] = argv[i++];
        }

        if (ntok == 0) {
            fprintf(stderr, "Empty command\n");
            return EXIT_FAILURE;
        }
        if (nprocs >= MAX_PROCESSES) {
            fprintf(stderr, "Too many processes (max %d)\n", MAX_PROCESSES);
            return EXIT_FAILURE;
        }

        procs[nprocs++] = process_create(tokens, ntok);

        if (i < argc && strcmp(argv[i], ":") == 0) {
            i++; /* skip delimiter */
            if (i >= argc) {
                fprintf(stderr, "Trailing ':' with no following command\n");
                return EXIT_FAILURE;
            }
        }
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigalrm;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        die_perror("sigaction SIGALRM");
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigchld;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        die_perror("sigaction SIGCHLD");
    }

    int live_count = 0;
    for (int p = 0; p < nprocs; p++) {
        pid_t pid = fork();
        if (pid < 0) {
            die_perror("fork");
        }
        if (pid == 0) {
            if (kill(getpid(), SIGSTOP) < 0) {
                _exit(127);
            }
            execvp(procs[p]->argv[0], procs[p]->argv);
            perror("execvp");
            _exit(127);
        }

        procs[p]->pid = pid;

        int st;
        pid_t r;
        do {
            r = waitpid(pid, &st, WUNTRACED);
        } while (r < 0 && errno == EINTR);

        if (r < 0) {
            die_perror("waitpid (initial stop)");
        }
        if (WIFSTOPPED(st)) {
            procs[p]->is_stopped = 1;
            live_count++;
        } else if (WIFEXITED(st) || WIFSIGNALED(st)) {
            process_destroy(&procs[p]);
        }
    }

    if (live_count == 0) {
        free(procs);
        return EXIT_SUCCESS;
    }

    sigset_t block_set, prev_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGALRM);
    sigaddset(&block_set, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &block_set, &prev_set) < 0) {
        die_perror("sigprocmask");
    }

    int cur = -1;
    while (live_count > 0) {
        cur = next_live_index((process_t *const *)procs, nprocs, cur);
        if (cur < 0) {
            break;
        }
        process_t *p = procs[cur];
        if (!p) {
            continue;
        }

        g_got_sigalrm = 0;
        g_got_sigchld = 0;

        set_quantum_timer(quantum_ms);
        if (kill(p->pid, SIGCONT) < 0) {
            cancel_timer();
            if (errno == ESRCH) {
                process_destroy(&procs[cur]);
                live_count--;
                continue;
            }
            die_perror("kill(SIGCONT)");
        }
        p->is_stopped = 0;

        for (;;) {
            while (!g_got_sigalrm && !g_got_sigchld) {
                sigsuspend(&prev_set);
            }

            if (g_got_sigchld) {
                g_got_sigchld = 0;
                reap_any_children(procs, nprocs, &live_count);

                if (!procs[cur] || procs[cur]->is_stopped) {
                    cancel_timer();
                    break;
                }
                continue;
            }

            if (g_got_sigalrm) {
                cancel_timer();
                reap_any_children(procs, nprocs, &live_count);
                break;
            }
        }

        if (g_got_sigalrm) {
            if (procs[cur] && procs[cur]->is_stopped == 0) {
                if (kill(procs[cur]->pid, SIGSTOP) < 0) {
                    if (errno != ESRCH) {
                        die_perror("kill(SIGSTOP)");
                    }
                } else {
                    int st;
                    pid_t r;
                    do {
                        r = waitpid(procs[cur]->pid, &st, WUNTRACED);
                    } while (r < 0 && errno == EINTR);

                    if (r > 0) {
                        handle_child_status(procs, nprocs, &live_count, r, st);
                    } else if (r < 0 && errno != ECHILD) {
                        die_perror("waitpid (preempt)");
                    }
                }
            }
        }

        reap_any_children(procs, nprocs, &live_count);
    }

    (void)sigprocmask(SIG_SETMASK, &prev_set, NULL);

    for (int p = 0; p < nprocs; p++) {
        process_destroy(&procs[p]);
    }
    free(procs);

    return EXIT_SUCCESS;
}
