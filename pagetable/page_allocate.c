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
        if(posix_memalign((void**)&ptbr, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) != 0) {
            fprintf(stderr, "posix_memalign failed");
            exit(127);
        }
        size_t *pt = (size_t *)ptbr;
        for(int i = 0; i < TOTAL_ENTRIES; i++)
        {
            pt[i] = 0;
        }
    }

    size_t vpn = va >> POBITS; // extract vpn
    size_t *cur_table = (size_t*)ptbr;

    // iterate through page table levels
    for(int i = 0; i < LEVELS - 1; i++) {
        int shift = (LEVELS - i - 1) * (INDEX_BITS);
        size_t index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        if(!(cur_table[index] & 1)) {
            void *newpt = NULL;

            if (posix_memalign((void **)&newpt, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) == 0) {
                size_t *newpt_ptr = (size_t *)newpt;

                for (size_t i = 0; i < TOTAL_ENTRIES; i++) {

                    newpt_ptr[i] = 0;
                }

                cur_table[index] = ((size_t)newpt & ~(PAGE_OFFSET_MASK)) | 1;
            } else {
                fprintf(stderr, "posix_memalign failed");
                exit(127);
            }
        }

        cur_table = (size_t*)((cur_table[index] & ~PAGE_OFFSET_MASK)); // next page table
    }

    size_t finalIndex = vpn & INDEX_MASK;

    if(!(cur_table[finalIndex] & 1)) {
    void *pp = NULL;
        if (posix_memalign((void **)&pp, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE) == 0) {
            size_t ppn = ((size_t)pp >> POBITS);

            cur_table[finalIndex] = (ppn << POBITS) | 1;
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
    size_t pageOffset = va & PAGE_OFFSET_MASK;

    for(int i = 0; i < LEVELS - 1; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        size_t index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level
        size_t pte = cur_table[index];

        if (!(pte & 1)) {

            return 0xFFFFFFFFFFFFFFFF;
        }
        cur_table = (size_t*)(pte & ~INDEX_MASK); // next page table
    }

    size_t finalIndex = vpn & PAGE_OFFSET_MASK;
    size_t pte = cur_table[finalIndex];
    if (!(pte & 1)) {
        return 0xFFFFFFFFFFFFFFFF;
    }  
    size_t ppn = pte >> POBITS;
    return (ppn << POBITS) | pageOffset;
}

void print_translation(size_t va) {
    size_t pa = translate(va);
    if (pa == 0xFFFFFFFFFFFFFFFF) {
        printf("VA 0x%lx -> Invalid (not allocated)\n", va);
    } else {
        printf("VA 0x%lx -> PA 0x%lx\n", va, pa);
    }
} 

int main() {
    // Test cases for page allocation and translation
    size_t test_addresses[] = {
        0x00000000,   // Edge case: beginning of address space
        0x00001000,   // Normal case: first page
        0x00002000,   // Normal case: second page
        0x00100000,   // Higher address: page 256
        0x00200000,   // Higher address: page 512
        0x3FFFFFFF,   // Edge case: near the end of address space
        0xFFFFFFFF    // Edge case: max possible address
    };

    size_t num_tests = sizeof(test_addresses) / sizeof(test_addresses[0]);

    // Allocate pages for the test addresses
    for (size_t i = 0; i < num_tests; i++) {
        printf("Allocating page for VA 0x%lx...\n", test_addresses[i]);
        page_allocate(test_addresses[i]);
    }

    // Test translations for the allocated pages
    printf("\nTesting translations:\n");
    for (size_t i = 0; i < num_tests; i++) {
        print_translation(test_addresses[i]);
    }

    // Test translating an unallocated address
    size_t unallocated_va = 0x00003000; // This address was never allocated
    printf("\nTesting translation for unallocated VA 0x%lx:\n", unallocated_va);
    print_translation(unallocated_va);

    // Test boundary conditions
    printf("\nTesting boundary translations:\n");
    print_translation(0x3FFFFFFF); // Near upper limit
    print_translation(0xFFFFFFFF);  // Upper limit

    return 0;
}