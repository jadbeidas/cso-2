CC = gcc
CFLAGS = -D_XOPEN_SOURCE=700 -Og -g -std=c11 -Wall -pedantic -fsanitize=address
LDFLAGS = -fsanitize=address


all: gettimings

gettimings: gettimings.c
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm --force gettimings

.PHONY: all clean