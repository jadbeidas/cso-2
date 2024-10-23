#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mlpt.h"
#include "config.h"

size_t ptbr = 0;
static size_t allocation_count = 0; // keep track of pages

#define PAGE_TABLE_SIZE (1UL << POBITS)
#define INDEX_BITS (POBITS - 3)
#define INDEX_MASK ((1UL << INDEX_BITS) - 1)
#define PAGE_OFFSET_MASK ((1UL << POBITS) - 1)

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

    if (LEVELS == 1) {
        index = vpn & INDEX_MASK;

        if ((cur_table[index] & 1) == 0) {
            cur_table[index] = (size_t)allocate_page();
            cur_table[index] |= 1;
        }
        return;
    }

    // iterate through page table levels
    for(int i = 0; i < LEVELS-1; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        if((cur_table[index] & 1) == 0) {
            // allocate and initialize new page table if necessary
            cur_table[index] = (size_t)allocate_page();
            cur_table[index] |= 1;
        }

        cur_table = (size_t*)((cur_table[index] & ~1)); // next page table
    }

    index = vpn & INDEX_MASK;
    if((cur_table[index] & 1) == 0) {
            cur_table[index] = (size_t)allocate_page();
            cur_table[index] |= 1;
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

    if (LEVELS == 1) {
        index = vpn & INDEX_MASK;
        if ((cur_table[index] & 1) == 0) {
            return 0xFFFFFFFFFFFFFFFF;
        }
        return ((size_t)cur_table[index] & ~1) | pageOffset;
    }

    for(int i = 0; i < LEVELS-1; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level
        if ((cur_table[index] & 1) == 0) {

            return 0xFFFFFFFFFFFFFFFF;
        }
        cur_table = (size_t*)(cur_table[index] & ~1); // next page table

    }

    size_t page_entry = cur_table[index];
    if (!(page_entry & 1)) {
        return 0xFFFFFFFFFFFFFFFF;
    }
    size_t physical_address = page_entry & ~1;
    return physical_address | pageOffset;

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