#ifndef _MEMORY_H
#define _MEMORY_H

#include "../lib/stdint.h"

#define PAGE_SIZE 4096

typedef struct {
    void *address;
    long length;
} MEM_phys_block;

/** @brief Node for the free list of page frames.
 */
typedef struct page_frame {
    void *address;
    struct page_frame *next;
} page_frame;

void MMU_pf_init();
void *MMU_pf_alloc(void);
int MMU_pf_free(void *pf);

#endif
