#define _GNU_SOURCE
#include "util.h"
#include <stdio.h>      // for printf
#include <stdlib.h>     // for atoi (and malloc() which you'll likely use)
#include <sys/mman.h>   // for mmap() which you'll likely use
#include <stdalign.h>

alignas(4096) volatile char global_array[4096 * 32];


void labStuff(int which) {
    if (which == 0) {
        /* do nothing */
    }
    else if (which == 1) {
        for (int i = 0; i < 32; i++) {
            global_array[i * 4096] = 'a';
        }
        for (int i = 0; i < 1000; i++) {
            global_array[i] = 'b';
        }
    } else if (which == 2) {
        volatile char *ptr = malloc(1024 * 1024);
        if (ptr) {
            ptr[0] = 'c';
            ptr[4096] = 'd';
            ptr[8192] = 'e';
        }
    } else if (which == 3) {
        char *ptr;
        ptr = mmap(NULL ,
                1048576 ,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1, 
                0
            );
        if (ptr == MAP_FAILED) { return; }
        for (int i = 0; i < 32; i++) {
            global_array[i * 4096] = 'f';
        }
    } else if (which == 4) {
        char *ptr;
        ptr = mmap((void*) (0x5555555bd000 + 0x200000),
                4096,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
                -1,
                    0
            );
        if (ptr == MAP_FAILED) { return; }

        for(int i = 0; i < 4096; i++)
        {
            ptr[i] = 'g';
        }
    } else if (which == 5) {
        char *ptr;
        ptr = mmap((void*) (0x5555555bd000 + 0x10000000000),
                4096,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
                -1,
                    0
            );
        if (ptr == MAP_FAILED) { return; }

        for(int i = 0; i < 4096; i++)
        {
            ptr[i] = 'h';
        }
    }
}

int main(int argc, char **argv) {
    int which = 0;
    if (argc > 1) {
        which = atoi(argv[1]);
    } else {
        fprintf(stderr, "Usage: %s NUMBER\n", argv[0]);
        return 1;
    }
    printf("Memory layout:\n");
    print_maps(stdout);
    printf("\n");
    printf("Initial state:\n");
    force_load();
    struct memory_record r1, r2;
    record_memory_record(&r1);
    print_memory_record(stdout, NULL, &r1);
    printf("---\n");

    printf("Running labStuff(%d)...\n", which);

    labStuff(which);

    printf("---\n");
    printf("Afterwards:\n");
    record_memory_record(&r2);
    print_memory_record(stdout, &r1, &r2);
    print_maps(stdout);
    return 0;
}
