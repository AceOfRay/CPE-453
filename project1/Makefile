CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = schedule two

.PHONY: all clean

all: $(TARGETS)

schedule: schedule.o
	$(CC) $(CFLAGS) -o schedule schedule.o

schedule.o: schedule.c schedule.h
	$(CC) $(CFLAGS) -c schedule.c

two: two.c
	$(CC) $(CFLAGS) -o two two.c

clean:
	rm -f $(TARGETS) *.o
