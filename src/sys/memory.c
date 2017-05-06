#include "memory.h"
#include "multiboot.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include <stddef.h>

/* Free page list. */
static page_frame *free_list_head;
/*
static uint8_t *next_page;
static uint8_t *cur_page;
*/

/* Address of next free physical page frame. */
static uint8_t *phys_blocks;

/* How physical bytes not in free list are left. */
static int untracked_bytes_left;

/* Physical low and high memory and kernel locations. */
static MB_mem_info mem_info;

void MMU_pf_init(MB_basic_tag *mb_tag) {

    free_list_head = NULL; /* Free list doesn't exist yet. */
    phys_blocks = (uint8_t *) mem_info.low.address + PAGE_SIZE;
    untracked_bytes_left = mem_info.low.size - PAGE_SIZE;

    printk("\nChecking multiboot tags\n");
    mem_info = MB_parse_tags(mb_tag);
    printk("low size %u\n", mem_info.low.size);
}

void *MMU_pf_alloc(void) {
    void *ret = NULL;
    uint64_t high_address = mem_info.high.address;

    /* Get page directly from memory. */
    if (phys_blocks) {
        /* Check if memory block is less than a full page. */
        if (untracked_bytes_left < PAGE_SIZE) {

            /* Check if referencing low memory. */
            if ((uint64_t) phys_blocks < mem_info.high.address) {
                untracked_bytes_left = mem_info.high.size;

                /* Check if high address is less than a page before kernel. */
                if (mem_info.high.address - mem_info.kern_start < PAGE_SIZE) {
                    high_address = mem_info.kern_start + mem_info.kern_size;

                    if (high_address % PAGE_SIZE) /* Round up to a page. */
                        high_address += PAGE_SIZE - (high_address % PAGE_SIZE);

                    /* Adjust size of untracked physical memory. */
                    untracked_bytes_left -= high_address - 
                     mem_info.high.address;
                }

                phys_blocks = (uint8_t *) high_address;
                ret = (void *) phys_blocks;
                phys_blocks += PAGE_SIZE;
                untracked_bytes_left -= PAGE_SIZE;
            }
            else
                ret = NULL;
        }
        else {
            ret = (void *) phys_blocks;
            phys_blocks += PAGE_SIZE;
            untracked_bytes_left -= PAGE_SIZE;
        }
    }
    else { /* Get page from free list. */
        /* TODO: Implement this after heap allocator has been completed. */
    }

    return ret;
}

int MMU_pf_free(void *pf) {
    uint64_t frame = (uint64_t) pf;

    if (frame % PAGE_SIZE) { /* Received a pointer that is 4K aligned. */
    }

    return EXIT_FAILURE;
}
