CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

TARGETS = who_runs_first fork_loop

all: $(TARGETS)

who_runs_first: who_runs_first.c
	$(CC) $(CFLAGS) -o who_runs_first who_runs_first.c

fork_loop: fork_loop.c
	$(CC) $(CFLAGS) -o fork_loop fork_loop.c

clean:
	rm -f $(TARGETS)

.PHONY: all clean