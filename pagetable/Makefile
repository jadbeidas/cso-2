CC = gcc
CFLAGS = -Wall
LDFLAGS = -static

all: libmlpt.a

libmlpt.a: page_allocate.o
	ar rcs -o libmlpt.a page_allocate.o

page_allocate.o:  page_allocate.c mlpt.h config.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c page_allocate.c -o page_allocate.o

clean:
	rm --force *.o libmlpt.a

.PHONY: all clean