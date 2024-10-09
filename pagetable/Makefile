CC = gcc
CFLAGS = -D_XOPEN_SOURCE=700 -Og -g -std=c11 -Wall -pedantic 
#-fsanitize=address
#LDFLAGS = -fsanitize=address

all: page_allocate

page_allocate.o:  page_allocate.c mlpt.h config.h
	$(CC) $(CFLAGS) -c page_allocate.c -o page_allocate.o

page_allocate: page_allocate.o
	$(CC) $(LDFLAGS) -o page_allocate page_allocate.o

clean:
	rm --force *.o page_allocate

.PHONY: all clean