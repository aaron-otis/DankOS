/**
 * @file
 */
#include "memory.h"
#include "multiboot.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../drivers/interrupts.h"
#include <stddef.h>

#define VIRT_ADDR_MASK 0x1FF /* 9 bits. */
#define BASE_ADDR_MASK ((1UL << 40) - 1)

#define PT_OFFSET_SHIFT 12
#define PD_OFFSET_SHIFT (PT_OFFSET_SHIFT + 9)
#define PDP_OFFSET_SHIFT (PD_OFFSET_SHIFT + 9)
#define PML4_OFFSET_SHIFT (PDP_OFFSET_SHIFT + 9)

static PML4 *page_map_l4;

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
    int ints_enabled = 0;
    page_frame *ptr;
    uint64_t address;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    mem_info = MB_parse_tags(mb_tag);
    if (mem_info.low.address == 0)
        free_list_head = (page_frame *) mem_info.low.address + PAGE_SIZE;
    else
        free_list_head = (page_frame *) mem_info.low.address;

    /* Populate free list with addresses from lower memory region. */
    ptr = free_list_head;
    while ((uint64_t) ptr < mem_info.low.address + mem_info.low.size - 
     PAGE_SIZE) {
        address = (uint64_t) ptr;
        ptr->next = (page_frame *) (address + PAGE_SIZE);
        ptr = ptr->next;
    }

    /* Set start of free memory not in free pool. */
    address = mem_info.high.address;
    if (address < mem_info.kern_start + mem_info.kern_size)
        address = mem_info.kern_start + mem_info.kern_size;

    if (address % PAGE_SIZE) /* Round up to PAGE_SIZE. */
        address += PAGE_SIZE - address % PAGE_SIZE;

    phys_blocks = (uint8_t *) address;
    untracked_bytes_left = mem_info.high.size;

    if (ints_enabled)
        STI;
}

/** @brief Allocates a page frame.
 *
 * Searches memory for a free page and returns it.
 * @returns A pointer to the start of a page frame or NULL if no more free
 * pages exist.
 * @pre Multiboot 2 type 6 (memory) and 9 (ELF sections) tags have been parsed 
 * and information stored in the global MB_mem_info struct.
 * @post A free page frame has been allocated and tracked.
 */
void *MMU_pf_alloc(void) {
    void *ret = NULL;
    int ints_enabled = 0;
    page_frame *pf;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    if (phys_blocks) { /* Get page directly from memory. */

        /* Check if memory block is less than a full page. */
        if (untracked_bytes_left >= PAGE_SIZE) {
            ret = (void *) phys_blocks;
            phys_blocks += PAGE_SIZE;
            untracked_bytes_left -= PAGE_SIZE;

        }
        else
            ret = NULL;

        if (untracked_bytes_left < PAGE_SIZE)
            phys_blocks = NULL;
    }
    else if (free_list_head) { /* Get page from free list. */
        if (free_list_head) {
            pf = free_list_head;
            free_list_head = free_list_head->next;
            ret = (void *) pf;
        }
    }
    else
        ret = NULL;

    if (ints_enabled)
        STI;

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
    page_frame *frame = pf;
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    /* Received a non-NULL pointer that is 4K aligned. */
    if (pf && !((uint64_t) frame % PAGE_SIZE)) {
        /* Add page frame to the free pool. */
        frame->next = free_list_head;
        free_list_head = frame;

        if (ints_enabled)
            STI;

        return EXIT_SUCCESS;
    }

    if (ints_enabled)
        STI;

    return EXIT_FAILURE;
}

static int walk_page_table() {
}

/** @brief Initializes virtual memory management.
 *
 * @post The virtual memory manager is initialized.
 */
int MMU_init() {
    int ints_enabled = 0;
    uint64_t phys_mem_size = mem_info.high.address + mem_info.high.size;
    uint64_t i, index;
    PDP *pdp;
    PD *pd;
    PT *pt;

    /* 
     * Need to:
     *  - Create at least one PDPT per region. 
     *  - Install the same PML4E entry in every address space.
     *  - Updates to PDPE or lower levels does not require changing any
     *    PML4 tables.
     *  - Cross-compiler will place user space programs at address 0x10000
     *    (where the kernel is located).
     */

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    /* Create level 4 page map. */
    page_map_l4 = MMU_pf_alloc();
    memset(page_map_l4, 0, PAGE_SIZE);

    for (i = 0; i < PAGE_SIZE / sizeof(page_map_l4); i++) {
        page_map_l4[i].r_w = 1;
    }

    /* Create at least one PDPT per region. */

    /* Identity map. */
    for (i = 0; i <= phys_mem_size; i += PAGE_SIZE) {
        index = (i >> PML4_OFFSET_SHIFT) & VIRT_ADDR_MASK;

        if (!page_map_l4[index].present) { /* Create PDP if not present. */
            page_map_l4[index].present = 1;
            page_map_l4[index].r_w = 1;

            pdp = MMU_pf_alloc();
            page_map_l4[index].base_addr = (uint64_t) pdp >> PT_OFFSET_SHIFT;
            memset(pdp, 0, PAGE_SIZE);

        }
        else /* PDP is present. */
            pdp = (PDP *) ((uint64_t) page_map_l4[index].base_addr << 
             PT_OFFSET_SHIFT);

        index = (i >> PDP_OFFSET_SHIFT) & VIRT_ADDR_MASK;
        if (!pdp[index].present) { /* Create PD if not present. */
            pd = MMU_pf_alloc();
            pdp[index].base_addr = (uint64_t) pd >> PT_OFFSET_SHIFT;

            pdp[index].present = 1;
            pdp[index].r_w = 1;
            memset(pd, 0, PAGE_SIZE);
        }
        else /* PD is present. */
            pd = (PD *) ((uint64_t) pdp[index].base_addr << PT_OFFSET_SHIFT);

        index = (i >> PD_OFFSET_SHIFT) & VIRT_ADDR_MASK;
        if (!pd[index].present) { /* Create page table if not present. */
            pt = MMU_pf_alloc();
            pd[index].base_addr = (uint64_t) pt >> PT_OFFSET_SHIFT;

            pd[index].present = 1;
            pt[index].r_w = 1;
            memset(pt, 0, PAGE_SIZE);
        }
        else /* Page table is present. */
            pt = (PT *) ((uint64_t) pd[index].base_addr << PT_OFFSET_SHIFT);

        if (i) /* Set current address to be the actual page referenced. */
            pt[index].base_addr = i >> PT_OFFSET_SHIFT;
        else /* Do not map address zero. */
            pt[index].base_addr = (uint64_t) MMU_pf_alloc() >> PT_OFFSET_SHIFT;


        printk("Page address: %p\n", pt[index]);
    }
    printk("Memory end: %qx\n", mem_info.high.address + mem_info.high.size);

    /* Kernel stacks. */

    /* Growth region. */

    /* Kernel heaps. */

    /* User space. */




    /* Set CR3 last. */

    if (ints_enabled)
        STI;
}
