CC = gcc
CFLAGS = -D_XOPEN_SOURCE=700 -Og -g -std=c11 -Wall -pedantic -fsanitize=address
LDFLAGS = -fsanitize=address

all: main

fork_run.o: fork_run.c
	$(CC) $(CFLAGS) -c fork_run.c -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o $@

main: fork_run.o main.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm --force *.o main fork_run.o main.o

.PHONY: all clean