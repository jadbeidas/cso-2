#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mlpt.h"
#include "config.h"

size_t ptbr = 0;
static size_t allocation_count = 0; // keep track of pages

#define INDEX_BITS ((64 - POBITS) / LEVELS)
#define INDEX_MASK ((1UL << INDEX_BITS) - 1)
#define PAGE_OFFSET_MASK ((1UL << POBITS) - 1)

// align and allocate memory for a new page
void *allocate_page() {
    void *page;
    if(posix_memalign(&page, 4096, 4096) != 0) {
        fprintf(stderr, "posix_memalign failed");
        exit(127);
    }
    allocation_count++;
    return page;
}

// initialize ptbr
static void set_testing_ptbr(void) {
    ptbr = (size_t) allocate_page();
    // initialize all page table entries to zero
    memset((void*)ptbr, 0, 4096);
}

// allocate a page for a specific virtual address
void page_allocate(size_t va) {
    if (ptbr == 0) {
        set_testing_ptbr();
    }

    size_t vpn = va >> POBITS; // extract vpn
    size_t *cur_table = (size_t*)ptbr;
    size_t index;
    // iterate through page table levels
    for(int i = 0; i < LEVELS; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        if((cur_table[index] & 1) == 0) {
            // allocate and initialize new page table if necessary
            cur_table[index] = (size_t)allocate_page();
            cur_table[index] = cur_table[index] | 1;
            memset((void*)(cur_table[index] & ~PAGE_OFFSET_MASK), 0, 4096);
        }

        cur_table = (size_t*)(cur_table[index] & ~PAGE_OFFSET_MASK); // next page table
    }

    index = (va >> POBITS) & INDEX_MASK;
    if((cur_table[index] & 1) == 0) {
            cur_table[index] = (size_t)allocate_page();
            cur_table[index] = cur_table[index] | 1;
    }
}

size_t translate(size_t va) {
    if (ptbr == 0) {
        return 0xFFFFFFFFFFFFFFFF;
    }

    size_t vpn = va >> POBITS; // extract vpn
    size_t index;
    size_t *cur_table = (size_t*) ptbr;
    for(int i = 0; i < LEVELS; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        if ((cur_table[index] & 1) == 0) {
        return 0xFFFFFFFFFFFFFFFF;
    }
        cur_table = (size_t*)(cur_table[index] & ~PAGE_OFFSET_MASK); // next page table
    }

    return (size_t)cur_table | (va & ((1 << POBITS) - 1));
}

/*
int main() {
    page_allocate(3 << POBITS);
    size_t *pointer_to_table;
    pointer_to_table = (size_t *) ptbr;
    size_t page_table_entry = pointer_to_table[3];
    printf("PTE @ index 3: valid bit=%d  physical page number=0x%lx\n",
        (int) (page_table_entry & 1),
        (long) (page_table_entry >> 12)
    );
} */