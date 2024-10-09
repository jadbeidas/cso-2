#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdalign.h>
#include <string.h>
#include "mlpt.h"
#include "config.h"

#define _XOPEN_SOURCE 700

size_t ptbr = 0;
static size_t allocation_count = 0; // keep track of pages

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
    memset((void*)ptbr, 0, 4096); // assuming 4096 bytes for the page table
}

// allocate a page for a specific virtual address
void page_allocate(size_t virtual_address) {
    if (ptbr == 0) {
        set_testing_ptbr();
    }

    size_t vpn = virtual_address >> POBITS; // extract vpn

    size_t *page_table = (size_t*)ptbr;
    size_t pte = page_table[vpn]; // check if page is allocated already

    // if the valid bit is not set, the page has not been allocated
    if (!(pte & 1)) {
        void *data_page = allocate_page(); // allocate data page
        size_t physical_page_number = (size_t)data_page >> POBITS; // get page number
        page_table[vpn] = (physical_page_number << 1) | 1; // set page table entry with valid bit
    }
}

int main() {
    page_allocate(3 << POBITS);
    size_t *pointer_to_table;
    pointer_to_table = (size_t *) ptbr;
    size_t page_table_entry = pointer_to_table[3];
    printf("PTE @ index 3: valid bit=%d  physical page number=0x%lx\n",
        (int) (page_table_entry & 1),
        (long) (page_table_entry >> 12)
    );
}