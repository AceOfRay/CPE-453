CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = schedule two

.PHONY: all clean

all: $(TARGETS)

schedule: schedule.o command_parser.o queue.o
	$(CC) $(CFLAGS) -o schedule schedule.o command_parser.o queue.o

schedule.o: schedule.c schedule.h command_parser.h
	$(CC) $(CFLAGS) -c schedule.c

command_parser.o: command_parser.c command_parser.h
	$(CC) $(CFLAGS) -c command_parser.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

two: two.c
	$(CC) $(CFLAGS) -o two two.c

clean:
	rm -f $(TARGETS) *.o
