#include <stddef.h>
#include <stdint.h>
#include "config.h"
#include "mlpt.h"
#include <assert.h>

#define PAGE_OFFSET_MASK ((1 << POBITS) - 1)
#define TLB_SETS 16
#define TLB_WAYS 4

typedef struct {
    size_t vpn;
    size_t ppn;
    int valid_bit;
    int lru_counter;
} TLBEntry;

typedef struct {
    TLBEntry entries[TLB_SETS][TLB_WAYS];
} TLB;

TLB tlb;

void update_lru(TLBEntry *set, int access_index) {
    for (int i = 0; i < TLB_WAYS; ++i) {
        if (set[i].valid_bit) {
            if (set[i].lru_counter < TLB_WAYS) {
                set[i].lru_counter++;
            }
        }
    }
    set[access_index].lru_counter = 1;
}

int find_lru_index(TLBEntry *set) {
    for (int i = 0; i < TLB_WAYS; ++i) {
        if (!set[i].valid_bit) {
            return i;
        }
    }

    int max_lru_index = 0;
    int max_lru_counter = set[0].lru_counter;

    for (int i = 1; i < TLB_WAYS; ++i) {
        if (set[i].lru_counter > max_lru_counter) {
            max_lru_counter = set[i].lru_counter;
            max_lru_index = i;
        }
    }
    return max_lru_index;
}

void tlb_clear() {
    for (int i = 0; i < TLB_SETS; ++i) {
        for (int j = 0; j < TLB_WAYS; ++j) {
            tlb.entries[i][j].valid_bit = 0;
            tlb.entries[i][j].lru_counter = 0; 
        }
    }
}

int tlb_peek(size_t va) {
    size_t vpn = va >> POBITS;
    size_t set_index = vpn % TLB_SETS;

    for (int i = 0; i < TLB_WAYS; ++i) {
        if (tlb.entries[set_index][i].valid_bit && tlb.entries[set_index][i].vpn == vpn) {
            return tlb.entries[set_index][i].lru_counter;
        }
    }
    return 0;
}

size_t tlb_translate(size_t va) {
    size_t vpn = va >> POBITS;
    size_t set_index = vpn % TLB_SETS;
    size_t page_aligned_va = va & ~PAGE_OFFSET_MASK;

    for (int i = 0; i < TLB_WAYS; ++i) {
        if (tlb.entries[set_index][i].valid_bit && tlb.entries[set_index][i].vpn == vpn) {
            update_lru(tlb.entries[set_index], i);
            return (tlb.entries[set_index][i].ppn << POBITS) | (va & PAGE_OFFSET_MASK);
        }
    }

    size_t ppn = translate(page_aligned_va);
    if (ppn == (size_t) - 1) {
        return -1;
    }

    int lru_index = find_lru_index(tlb.entries[set_index]);
    tlb.entries[set_index][lru_index].vpn = vpn;
    tlb.entries[set_index][lru_index].ppn = ppn >> POBITS;
    tlb.entries[set_index][lru_index].valid_bit = 1;

    update_lru(tlb.entries[set_index], lru_index);
    return (ppn & ~PAGE_OFFSET_MASK) | (va & PAGE_OFFSET_MASK);
}
/*
/** stub for the purpose of testing tlb_* functions 
size_t translate(size_t va) {
    if (va < 0x1234000)
        return va + 0x20000;
    else if (va > 0x2000000 && va < 0x2345000)
        return va + 0x100000;
    else
        return -1;
}

int main() {
    tlb_clear();
    assert(tlb_peek(0) == 0);
    assert(tlb_translate(0) == 0x0020000);
    assert(tlb_peek(0) == 1);
    assert(tlb_translate(0x200) == 0x20200);
    assert(tlb_translate(0x400) == 0x20400);
    assert(tlb_peek(0) == 1);
    assert(tlb_peek(0x200) == 1);
    assert(tlb_translate(0x2001200) == 0x2101200);
    assert(tlb_translate(0x0005200) == 0x0025200);
    assert(tlb_translate(0x0008200) == 0x0028200);
    assert(tlb_translate(0x0002200) == 0x0022200);
    assert(tlb_peek(0x2001000) == 1);
    assert(tlb_peek(0x0001000) == 0);
    assert(tlb_peek(0x0004000) == 0);
    assert(tlb_peek(0x0005000) == 1);
    assert(tlb_peek(0x0008000) == 1);
    assert(tlb_peek(0x0002000) == 1);
    assert(tlb_peek(0x0000000) == 1);
    tlb_clear();
    assert(tlb_peek(0x2001000) == 0);
    assert(tlb_peek(0x0005000) == 0);
    assert(tlb_peek(0x0008000) == 0);
    assert(tlb_peek(0x0002000) == 0);
    assert(tlb_peek(0x0000000) == 0);
    assert(tlb_translate(0) == 0x20000);
    assert(tlb_peek(0) == 1);

    tlb_clear();
    assert(tlb_translate(0x0001200) == 0x0021200);
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x0801200) == 0x0821200);
    assert(tlb_translate(0x2301200) == 0x2401200);
    assert(tlb_translate(0x0501200) == 0x0521200);
    assert(tlb_translate(0x0A01200) == 0x0A21200);
    assert(tlb_peek(0x0001200) == 0);
    assert(tlb_peek(0x2101200) == 0);
    assert(tlb_peek(0x2301200) == 3);
    assert(tlb_peek(0x0501200) == 2);
    assert(tlb_peek(0x0801200) == 4);
    assert(tlb_peek(0x0A01200) == 1);
    assert(tlb_translate(0x2301800) == 0x2401800);
    assert(tlb_peek(0x0001000) == 0);
    assert(tlb_peek(0x2101000) == 0);
    assert(tlb_peek(0x2301000) == 1);
    assert(tlb_peek(0x0501000) == 3);
    assert(tlb_peek(0x0801000) == 4);
    assert(tlb_peek(0x0A01000) == 2);
    assert(tlb_translate(0x404000) == 0x424000);
    tlb_clear();
    assert(tlb_peek(0x301000) == 0);
    assert(tlb_peek(0x501000) == 0);
    assert(tlb_peek(0x801000) == 0);
    assert(tlb_peek(0xA01000) == 0);
    assert(tlb_translate(0xA01200) == 0xA21200);

    tlb_clear();
    assert(tlb_translate(0xA0001200) == -1);
    assert(tlb_peek(0xA0001000) == 0);
    assert(tlb_translate(0x1200) == 0x21200);
    assert(tlb_peek(0xA0001200) == 0);
    assert(tlb_peek(0x1000) == 1);
    assert(tlb_translate(0xA0001200) == -1);
    assert(tlb_translate(0xB0001200) == -1);
    assert(tlb_translate(0xC0001200) == -1);
    assert(tlb_translate(0xD0001200) == -1);
    assert(tlb_translate(0xE0001200) == -1);
    assert(tlb_peek(0x1000) == 1);
    assert(tlb_translate(0x1200) == 0x21200);

    tlb_clear();
    assert(tlb_translate(0x0001200) == 0x0021200);
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x0801200) == 0x0821200);
    assert(tlb_translate(0x2301200) == 0x2401200);
    tlb_clear();
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x0001200) == 0x0021200);
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x2301200) == 0x2401200);
    assert(tlb_translate(0x0011200) == 0x0031200);
    assert(tlb_peek(0x0001200) == 4);
    assert(tlb_peek(0x2101200) == 3);
    assert(tlb_peek(0x2301200) == 2);
    assert(tlb_peek(0x0011200) == 1);
} */