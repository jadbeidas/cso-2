# Partners: Malik Kurtz (kra8ku), Martin Desousa (crx7jg)
CC = gcc
CFLAGS = -Wall
LDFLAGS = -Wall

all: guesser

cheer.o: cheer.c say.h
	$(CC) $(CFLAGS) -c cheer.c -o $@

grunt.o: grunt.c say.h
	$(CC) $(CFLAGS) -c grunt.c -o $@

guesser.o: guesser.c say.h
	$(CC) $(CFLAGS) -c guesser.c -o $@

libsay.a: cheer.o grunt.o
	ar rcs $@ $^

guesser: guesser.o libsay.a
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm --force *.o guesser libsay.a

.PHONY: all clean