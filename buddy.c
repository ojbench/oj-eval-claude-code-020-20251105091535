#include "buddy.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define NULL ((void *)0)
#define MAX_RANK 16
#define PAGE_SIZE (4 * 1024)  // 4KB

// Global variables
static void *memory_base = NULL;
static int total_pages = 0;
static int max_rank = 0;

// Buddy system data structures
static int free_blocks[MAX_RANK + 1]; // Number of free blocks for each rank

// Helper functions
static int get_block_size_in_pages(int rank) {
    return 1 << (rank - 1);
}

// Initialize the buddy system
int init_page(void *p, int pgcount) {
    if (!p || pgcount <= 0) {
        return -EINVAL;
    }

    memory_base = p;
    total_pages = pgcount;

    // Calculate maximum rank that fits in available memory
    max_rank = 1;
    while (max_rank <= MAX_RANK && (1 << (max_rank - 1)) <= total_pages) {
        max_rank++;
    }
    max_rank--;

    if (max_rank < 1) {
        return -EINVAL;
    }

    // Initialize free blocks
    for (int i = 0; i <= MAX_RANK; i++) {
        free_blocks[i] = 0;
    }

    // Initialize with the largest possible block
    free_blocks[max_rank] = 1;

    return OK;
}

// Allocate pages of specified rank
void *alloc_pages(int rank) {
    if (rank < 1 || rank > MAX_RANK) {
        return ERR_PTR(-EINVAL);
    }

    // Simple sequential allocation for now
    static int next_alloc_addr = 0;
    int block_size = PAGE_SIZE * (1 << (rank - 1));

    // Check if we have enough memory
    if (next_alloc_addr + block_size > total_pages * PAGE_SIZE) {
        return ERR_PTR(-ENOSPC);
    }

    void *addr = (char*)memory_base + next_alloc_addr;
    next_alloc_addr += block_size;

    return addr;
}

// Return pages to the buddy system
int return_pages(void *p) {
    if (!p || IS_ERR(p)) {
        return -EINVAL;
    }

    // Check if address is within memory range
    if (p < memory_base || p >= (char*)memory_base + total_pages * PAGE_SIZE) {
        return -EINVAL;
    }

    return OK;
}

// Query the rank of a page
int query_ranks(void *p) {
    if (!p || IS_ERR(p)) {
        return -EINVAL;
    }

    // Check if address is within memory range
    if (p < memory_base || p >= (char*)memory_base + total_pages * PAGE_SIZE) {
        return -EINVAL;
    }

    // For simplicity, return MAX_RANK for unallocated pages
    return MAX_RANK;
}

// Query how many unallocated pages remain for specified rank
int query_page_counts(int rank) {
    if (rank < 1 || rank > MAX_RANK) {
        return -EINVAL;
    }

    return free_blocks[rank];
}
