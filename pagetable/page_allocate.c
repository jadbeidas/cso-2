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

    printf("Allocating page for VA 0x%lx, VPN 0x%lx\n", va, vpn); // Debugging statement


    if (LEVELS == 1) {
        index = vpn & INDEX_MASK;
        printf("Level 1: Index 0x%lx\n", index); // Debugging statement

        if ((cur_table[index] & 1) == 0) {
            cur_table[index] = (size_t)allocate_page();
            printf("Allocated new page at index 0x%lx, cur_table[%lx] = 0x%lx\n", index, index, cur_table[index]); // Debugging statement

            memset((void *)(cur_table[index] & ~PAGE_OFFSET_MASK), 0, 4096);
            cur_table[index] |= 1;
        }
        return;
    }

    // iterate through page table levels
    for(int i = 0; i < LEVELS; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level

        printf("Level %d: Shift %d, Index 0x%lx, cur_table = %p\n", i, shift, index, (void*)cur_table); // Debugging statement
        if((cur_table[index] & 1) == 0) {
            // allocate and initialize new page table if necessary
            cur_table[index] = (size_t)allocate_page();

            printf("Allocated new page table for level %d at index 0x%lx, cur_table[%lx] = 0x%lx\n", i, index, index, cur_table[index]); // Debugging statement

            memset((void*)(cur_table[index] & ~PAGE_OFFSET_MASK), 0, 4096);
            cur_table[index] |= 1;
        }

        cur_table = (size_t*)(cur_table[index] & ~PAGE_OFFSET_MASK); // next page table
        printf("Moving to next level: cur_table = %p\n", (void*)cur_table); // Debugging statement

    }

    index = (va >> POBITS) & INDEX_MASK;
    printf("Final level index: 0x%lx\n", index); // Debugging statement

    if((cur_table[index] & 1) == 0) {
            cur_table[index] = (size_t)allocate_page();
            printf("Allocated page at final level: cur_table[%lx] = 0x%lx\n", index, cur_table[index]); // Debugging statement

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

    printf("Translating VA 0x%lx, VPN 0x%lx\n", va, vpn); // Debugging statement


    if (LEVELS == 1) {
        index = vpn & INDEX_MASK;
        printf("Level 1: Index 0x%lx\n", index); // Debugging statement

        if ((cur_table[index] & 1) == 0) {
            return 0xFFFFFFFFFFFFFFFF;
        }
        return ((size_t)cur_table[index] & ~1) | (va & PAGE_OFFSET_MASK);
    }

    for(int i = 0; i < LEVELS; i++) {
        int shift = (LEVELS - i - 1) * INDEX_BITS;
        index = (vpn >> shift) & INDEX_MASK; // extracting the index bits respective for this level
        printf("Level %d: Shift %d, Index 0x%lx, cur_table = %p\n", i, shift, index, (void *)cur_table); // Debugging statement

        if ((cur_table[index] & 1) == 0) {
            printf("Page table not found at level %d, index 0x%lx\n", i, index); // Debugging statement

            return 0xFFFFFFFFFFFFFFFF;
        }
        cur_table = (size_t*)(cur_table[index] & ~PAGE_OFFSET_MASK); // next page table
        printf("Moving to next level: cur_table = %p\n", (void *)cur_table); // Debugging statement

    }

    index = (vpn & INDEX_MASK);
    printf("Final level index: 0x%lx\n", index); // Debugging statement

    if ((cur_table[index] & 1) == 0) {
        printf("Page not found at final index 0x%lx\n", index); // Debugging statement

        return 0xFFFFFFFFFFFFFFFF;
    }

    return ((size_t)cur_table & ~1) | (va & PAGE_OFFSET_MASK);

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