CC = gcc
CFLAGS = -Wall
LDFLAGS = -static

all: libtlb.a

# Create the static library libtlb.a from the TLB object file
libtlb.a: tlb.o
	ar rcs -o libtlb.a tlb.o

# Compile tlb.o from tlb.c with the necessary headers
tlb.o: tlb.c tlb.h mlpt.h config.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c tlb.c -o tlb.o

# Clean up object files and the library
clean:
	rm --force *.o libtlb.a

.PHONY: all clean
