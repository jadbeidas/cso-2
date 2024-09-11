CC = gcc
CFLAGS = -D_XOPEN_SOURCE=700 -Og -g -std=c11 -Wall -pedantic -fsanitize=address
LDFLAGS = -fsanitize=address


all: split

split.o: split.c split.h
	$(CC) $(CFLAGS) -c split.c -o $@

main.o: main.c split.h
	$(CC) $(CFLAGS) -c main.c -o $@

split: main.o split.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm --force *.o split split.o main.o

.PHONY: all clean