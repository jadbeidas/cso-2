#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mlpt.h"
#include "config.h"

size_t ptbr = 0;

#define INDEX_BITS (POBITS - 3)
#define INDEX_MASK ((1UL << INDEX_BITS) - 1)
#define PAGE_OFFSET_MASK ((1UL << POBITS) - 1)

#define PAGE_TABLE_SIZE (1UL << POBITS)
#define ENTRY_SIZE 8
#define TOTAL_ENTRIES (PAGE_TABLE_SIZE / ENTRY_SIZE)

// align and allocate memory for a new page
void *allocate_page() {
    size_t *page;
    if(posix_memalign((void**)&page, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) != 0) {
        fprintf(stderr, "posix_memalign failed");
        exit(127);
    }
    for(int i = 0; i < TOTAL_ENTRIES; i++)
    {
        page[i] = 0;
    }
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

    // iterate through page table levels
    for(int i = 0; i < LEVELS - 1; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        size_t index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        if((cur_table[index] & 1) == 0) {
            void *newpt = NULL;

            if (posix_memalign((void **)&newpt, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) == 0) {
                size_t *newpt_ptr = (size_t *)newpt;

                for (size_t i = 0; i < TOTAL_ENTRIES; i++) {

                    newpt_ptr[i] = 0; // initialize everything to 0
                }

                cur_table[index] = ((size_t)newpt & ~(PAGE_OFFSET_MASK)) | 1; // store the page table and set its valid bit
            } else {
                fprintf(stderr, "posix_memalign failed");
                exit(127);
            }
        }

        cur_table = (size_t*)((cur_table[index] & ~PAGE_OFFSET_MASK)); // next page table
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

    for(int i = 0; i < LEVELS - 1; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level
        size_t pte = cur_table[index];

        if ((pte & (size_t) 1) == 0) {

            return 0xFFFFFFFFFFFFFFFF;
        }
        cur_table = (size_t*)(pte & ~INDEX_MASK); // next page table
    }

    size_t finalIndex = vpn & PAGE_OFFSET_MASK;
    size_t pte = cur_table[finalIndex];
    if ((pte & (size_t) 1) == 0) {
        return 0xFFFFFFFFFFFFFFFF;
    }  
    size_t ppn = pte >> POBITS;
    return (ppn << POBITS) | pageOffset;
}