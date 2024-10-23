#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mlpt.h"
#include "config.h"

size_t ptbr = 0;
static size_t allocation_count = 0; // keep track of pages

#define INDEX_BITS (POBITS - 3)
#define INDEX_MASK ((1UL << INDEX_BITS) - 1)
#define PAGE_OFFSET_MASK ((1UL << POBITS) - 1)

#define PAGE_TABLE_SIZE (1UL << POBITS)

// align and allocate memory for a new page
void *allocate_page() {
    void *page;
    if(posix_memalign((void**)&page, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) != 0) {
        fprintf(stderr, "posix_memalign failed");
        exit(127);
    }
    allocation_count++;
    memset(page, 0, PAGE_TABLE_SIZE);
    return page;
}

// initialize ptbr
static void set_testing_ptbr(void) {
    ptbr = (size_t) allocate_page();
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
    for(int i = 1; i <= LEVELS; i++) {
        int shift = (LEVELS - i) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        if((cur_table[index] & 1) == 0) {
            // allocate and initialize new page table if necessary
            cur_table[index] = (size_t)allocate_page() | 1;
        }

        cur_table = (size_t*)((cur_table[index] & ~1)); // next page table
    }
}

size_t translate(size_t va) {
    if (ptbr == 0) {
        return 0xFFFFFFFFFFFFFFFF;
    }

    size_t vpn = va >> POBITS; // extract vpn
    size_t index;
    size_t *cur_table = (size_t*) ptbr;
    size_t pageOffset = va & PAGE_OFFSET_MASK;

    for(int i = 1; i <= LEVELS; i++) {
        int shift = (LEVELS - i) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level
        
        size_t pte = cur_table[index];

        if ((pte & (size_t) 1) == 0) {

            return 0xFFFFFFFFFFFFFFFF;
        }
        cur_table = (size_t*)(pte & INDEX_MASK); // next page table
    }

    return (size_t)cur_table + pageOffset;

}