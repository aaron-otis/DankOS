/**
 * @file
 */
#include "memory.h"
#include "multiboot.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include <stddef.h>

/** @brief Pointer the head of free list. */
static page_frame *free_list_head;

/** @brief Address of next free physical page frame. */
static uint8_t *phys_blocks;

/** @brief How physical bytes not in free list are left. */
static int untracked_bytes_left;

/** @brief Physical low and high memory and kernel locations. */
static MB_mem_info mem_info;

/** @brief Initializes page frame allocator.
 * 
 * Initializes page frame allocator.
 * @param mb_tag a pointer to the beginning of the multiboot 2 tags.
 * @pre Multiboot 2 pointer must be valid.
 * @post The page frame allocator is ready to allocate page frames.
 */
void MMU_pf_init(MB_basic_tag *mb_tag) {

    free_list_head = NULL; /* Free list doesn't exist yet. */
    phys_blocks = (uint8_t *) mem_info.low.address + PAGE_SIZE;
    untracked_bytes_left = mem_info.low.size - PAGE_SIZE;

    printk("\nChecking multiboot tags\n");
    mem_info = MB_parse_tags(mb_tag);
    printk("low size %u\n", mem_info.low.size);
}

/** @brief Allocates a page frame.
 *
 * Searches memory for a free page and returns it.
 * @returns A pointer to the start of a page frame or NULL if no more free
 * pages exist.
 * @pre Multiboot 2 type 6 (memory) and 9 (ELF sections) tags have been parsed and information stored in the global MB_mem_info struct.
 * @post A free page frame has been allocated and tracked.
 */
void *MMU_pf_alloc(void) {
    void *ret = NULL;
    uint64_t high_address = mem_info.high.address;
    page_frame *pf;

    if (phys_blocks) { /* Get page directly from memory. */
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
        if (free_list_head) {
            pf = free_list_head;
            free_list_head = free_list_head->next;
            ret = pf->address;

            /* TODO: Free pf! */
        }
    }

    return ret;
}

/** @brief Frees a page frame.
 *
 * Frees a page frame and stores it in the free list.
 *
 * @param pf a pointer to the page frame to be freed.
 * @pre A page frame has been allocated.
 * @post pf is added to the free list.
 */
int MMU_pf_free(void *pf) {
    uint64_t frame = (uint64_t) pf;

    if (frame % PAGE_SIZE) { /* Received a pointer that is 4K aligned. */
    }

    return EXIT_FAILURE;
}
