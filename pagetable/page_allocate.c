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

// allocate a page for a specific virtual address
void page_allocate(size_t va) {
    if (ptbr == 0) {
        if (posix_memalign((void **)&ptbr, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) != 0) {
            fprintf(stderr, "posix_memalign failed");
            exit(127);
        }
        size_t *pt = (size_t *)ptbr;
        for (int i = 0; i < TOTAL_ENTRIES; ++i)
        {
            pt[i] = 0;
        }
    }

    size_t vpn = va >> POBITS; // extract vpn
    size_t *cur_table = (size_t*)ptbr;

    // iterate through page table levels
    for (int i = 0; i < LEVELS - 1; ++i) {
        int shift = (LEVELS - i - 1) * (INDEX_BITS);
        size_t index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        if (!(cur_table[index] & 1)) {
            void *newpt = NULL;

            if (posix_memalign((void **)&newpt, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) == 0) {
                size_t *newpt_ptr = (size_t *)newpt;

                for (size_t i = 0; i < TOTAL_ENTRIES; ++i) {

                    newpt_ptr[i] = 0;
                }

                cur_table[index] = ((size_t)newpt & ~(PAGE_OFFSET_MASK)) | 1;
            } else {
                fprintf(stderr, "posix_memalign failed");
                exit(127);
            }
        }

        cur_table = (size_t *)((cur_table[index] & ~PAGE_OFFSET_MASK)); // next page table
    }

    size_t final_index = vpn & INDEX_MASK;

    if (!(cur_table[final_index] & 1)) {
    void *pp = NULL;
        if (posix_memalign((void **)&pp, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) == 0) {
            size_t ppn = ((size_t)pp >> POBITS);

            cur_table[final_index] = (ppn << POBITS) | 1;
        } else {
            fprintf(stderr, "posix_memalign failed");
            exit(127);
        }
    }
}

size_t translate(size_t va) {
    if (ptbr == 0) {
        return 0xFFFFFFFFFFFFFFFF;
    }

    size_t vpn = va >> POBITS; // extract vpn
    size_t *cur_table = (size_t*) ptbr;
    size_t page_offset = va & PAGE_OFFSET_MASK;

    for (int i = 0; i < LEVELS - 1; ++i) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        size_t index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level
        size_t pte = cur_table[index];

        if (!(pte & 1)) {

            return 0xFFFFFFFFFFFFFFFF;
        }
        cur_table = (size_t*)(pte & ~INDEX_MASK); // next page table
    }

    size_t final_index = vpn & INDEX_MASK;
    size_t pte = cur_table[final_index];
    if (!(pte & 1)) {
        return 0xFFFFFFFFFFFFFFFF;
    }  
    size_t ppn = pte >> POBITS;
    return (ppn << POBITS) | page_offset;
}