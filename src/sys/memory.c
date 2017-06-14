/**
 * @file
 */
#include "memory.h"
#include "multiboot.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/debug.h"
#include "../drivers/interrupts.h"
#include <stddef.h>

#define VIRT_ADDR_MASK 0x1FF /* 9 bits. */
#define BASE_ADDR_MASK ((1UL << 40) - 1)
#define ALLOC_ON_DEMAND 1
#define PT_TRAVERSAL_ERROR -1
#define STACK_ALLIGN 0x10

#define PT_OFFSET_SHIFT 12
#define PD_OFFSET_SHIFT (PT_OFFSET_SHIFT + 9)
#define PDP_OFFSET_SHIFT (PD_OFFSET_SHIFT + 9)
#define PML4_OFFSET_SHIFT (PDP_OFFSET_SHIFT + 9)

/** @brief Pointer to level 4 entry of page table. */
static PML4 *page_map_l4;

/** @brief Pointer the head of free list. */
static page_frame *free_list_head;

/** @brief Address of next free physical page frame. */
static uint8_t *phys_blocks;

/** @brief How physical bytes not in free list are left. */
static int untracked_bytes_left;

/** @brief Physical low and high memory and kernel locations. */
static MB_mem_info mem_info;

/** @brief Tracks next heap address. */
static uint64_t next_virtual_address;

/** @brief Tracks next kernel stack address. */
static uint64_t next_kernel_stack;

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

/** @brief Walks the page table.
 *
 * @returns 
 * @pre PML4 must exist.
 * @post 
 */
static int walk_page_table(uint64_t addr, PML4 *pml4, PT **pt) {
    uint64_t index;
    int ints_enabled = 0;
    PDP *pdp;
    PD *pd;

    if (!pml4)
        return -1;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    index = (addr >> PML4_OFFSET_SHIFT) & VIRT_ADDR_MASK;
    if (!pml4[index].present) { /* Create PDP if not present. */
        pml4[index].present = 1;
        pml4[index].r_w = 1;

        pdp = MMU_pf_alloc();
        if (pdp == NULL) {
            printk("MMU_pf_alloc failed to alloc a PDP\n");
            HALT_CPU
        }
        pml4[index].base_addr = (uint64_t) pdp >> PT_OFFSET_SHIFT;
        memset(pdp, 0, PAGE_SIZE);
    }
    else /* PDP is present. */
        pdp = (PDP *) ((uint64_t) page_map_l4[index].base_addr << 
         PT_OFFSET_SHIFT);

    index = (addr >> PDP_OFFSET_SHIFT) & VIRT_ADDR_MASK;
    if (!pdp[index].present) { /* Create PD if not present. */
        pd = MMU_pf_alloc();
        if (pd == NULL) {
            printk("MMU_pf_alloc failed to alloc a PD\n");
            HALT_CPU
        }
        pdp[index].base_addr = (uint64_t) pd >> PT_OFFSET_SHIFT;

        pdp[index].present = 1;
        pdp[index].r_w = 1;
        memset(pd, 0, PAGE_SIZE);
    }
    else /* PD is present. */
        pd = (PD *) ((uint64_t) pdp[index].base_addr << PT_OFFSET_SHIFT);

    index = (addr >> PD_OFFSET_SHIFT) & VIRT_ADDR_MASK;
    if (!pd[index].present) { /* Create page table if not present. */
        *pt = MMU_pf_alloc();
        if (*pt == NULL) {
            printk("MMU_pf_alloc failed to alloc a PT\n");
            HALT_CPU
        }

        pd[index].base_addr = (uint64_t) *pt >> PT_OFFSET_SHIFT;
        pd[index].present = 1;
        pd[index].r_w = 1;
        memset(*pt, 0, PAGE_SIZE);
    }
    else { /* Page table is present. */
        *pt = (PT *) ((uint64_t) pd[index].base_addr << PT_OFFSET_SHIFT);
    }

    if (ints_enabled)
        STI;

    return (addr >> PT_OFFSET_SHIFT) & VIRT_ADDR_MASK;
}

/** @brief Initializes virtual memory management.
 *
 * @post The virtual memory manager is initialized.
 */
int MMU_init() {
    int ints_enabled = 0;
    const uint64_t phys_mem_size = mem_info.high.address + mem_info.high.size;
    uint64_t i, index;
    CR3 cr3;
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
    if (!page_map_l4) {
        printk("MMU_pf_alloc error allocating PML4\n");
        HALT_CPU
    }
    memset(page_map_l4, 0, PAGE_SIZE);

    /* Create at least one PDPT per region. */

    /* Identity map. */
    for (i = 0; i < phys_mem_size; i += PAGE_SIZE) {
        index = walk_page_table(i, page_map_l4, &pt);
        pt[index].present = 1;
        pt[index].r_w = 1;

        if (i) /* Set current address to be the actual page referenced. */
            pt[index].base_addr = i >> PT_OFFSET_SHIFT;
        else /* Do not map address zero. */
            pt[index].base_addr = (uint64_t) MMU_pf_alloc() >> PT_OFFSET_SHIFT;
    }

    /* Kernel stacks. */
    next_kernel_stack = KSTACKS_ADDR;

    /* Growth region. */

    /* Kernel heaps. */
    next_virtual_address = KHEAP_ADDR; /* Set first kernel heap address. */


    /* User space. */

    /* Set CR3 last. */
    cr3.base_addr = (uint64_t) page_map_l4 >> PT_OFFSET_SHIFT;
    cr3.reserved1 = 0;
    cr3.reserved2 = 0;
    cr3.reserved3 = 0;
    __asm__("movq %0, %%cr3" : : "r"(cr3));

    /* Set IRQ handler. */
    IRQ_set_handler(PAGE_FAULT, MMU_page_fault_handler, NULL);

    if (ints_enabled)
        STI;

    return EXIT_SUCCESS;
}

/* Kernel heap functions. */
void *MMU_alloc_page() {
    PT *pt;
    uint64_t index = walk_page_table(next_virtual_address, page_map_l4, &pt);
    void *ret;
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    pt[index].avl = ALLOC_ON_DEMAND; /* Set on demand allocation bit. */
    pt[index].present = 0; /* Will mark present after handler allocs page.  */

    ret = (void *) next_virtual_address;
    next_virtual_address += PAGE_SIZE;

    if (ints_enabled)
        STI;

    return ret;
}

void *MMU_alloc_pages(unsigned int num) {
    void *ret;
    int i, ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    ret = MMU_alloc_page();
    for (i = 1; i < num; i++)
        MMU_alloc_page();

    if (ints_enabled)
        STI;

    return ret;
}

void MMU_free_page(void *page) {
    PT *pt;
    uint64_t index, addr;
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    index = walk_page_table((uint64_t) page, page_map_l4, &pt);
    addr = (pt[index].base_addr << PT_OFFSET_SHIFT);
    MMU_pf_free((void *) addr);

    pt[index].present = 0;
    pt[index].avl &= !ALLOC_ON_DEMAND;

    if (ints_enabled)
        STI;
}

void MMU_free_pages(void *page, unsigned int num) {
    PT *pt;
    uint64_t index, addr = (uint64_t) page;
    int i, ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    for (i = 0; i < num; i++, addr += PAGE_SIZE)
        MMU_free_page((void *) addr);

    if (ints_enabled)
        STI;
}

/* Page fault handler. */
extern void MMU_page_fault_handler(int irq, int error, void *arg) {
    CR3 cr3;
    PML4 *pml4;
    PT *pt;
    int index;
    uint64_t mem_addr;

    /* Get PML4 address. */
    __asm__("movq %%cr3, %0" : "=r"(mem_addr));
    memcpy(&cr3, &mem_addr, sizeof(CR3));
    pml4 = (PML4 *) ((uint64_t) cr3.base_addr << PT_OFFSET_SHIFT);

    /* Get offending memory address. */
    __asm__("movq %%cr2, %0" : "=r"(mem_addr));

    /* Find level 1 table. */
    index = walk_page_table(mem_addr, pml4, &pt);

    if (pt[index].avl & ALLOC_ON_DEMAND) {
        pt[index].base_addr = (uint64_t) MMU_pf_alloc() >> PT_OFFSET_SHIFT;
        pt[index].present = 1;
        pt[index].avl &= !ALLOC_ON_DEMAND;

        if (pt[index].base_addr == 0) { /* Check if MMU_pf_alloc failed. */
            printk("MMU_pf_alloc failed in  MMU_page_fault_handler!\n");
            HALT_CPU
        }
    }
    else {
        printk("Error in MMU_page_fault_handler handling address %p\n", 
         (void *) mem_addr);
        HALT_CPU
    }
}

void *kbrk(intptr_t increment) {
    uint64_t size;
    int remainder;
    void *ret;

    if (!increment)
        return (void *) next_virtual_address;
    else if (increment < 0) {
        printk("kbrk error: increment %p\n", increment);
        return (void *) -1;
    }

    size = (uint64_t) increment / PAGE_SIZE;
    remainder = (uint64_t) increment % PAGE_SIZE;

    if (remainder)
        size++;

    ret = MMU_alloc_pages(size);
    if (!ret)
        ret = (void *) -1;

    return ret;
}

void *MMU_alloc_kstack() {
    PT *pt;
    uint64_t index;
    void *ret;
    int ints_enabled = 0;
    unsigned long long i, stack_end;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    stack_end = next_kernel_stack + KSTACK_SIZE;
    for (i = next_kernel_stack; i < stack_end; i += PAGE_SIZE) {
        index = walk_page_table(i, page_map_l4, &pt);
        pt[index].avl = ALLOC_ON_DEMAND; /* Set on demand allocation bit. */
        pt[index].present = 0;
    }

    if (ints_enabled)
        STI;
    /* Advance to next stack since stacks grow downward. */
    next_kernel_stack += KSTACK_SIZE;

    /* Make sure stack aligned and before the next stack. */
    ret = (void *) next_kernel_stack - STACK_ALLIGN;

    return ret;
}

void MMU_free_kstack(void *ptr) {
    uint64_t addr = (uint64_t) ptr;
    int pages;

    addr -= KSTACK_SIZE - STACK_ALLIGN;
    pages = KSTACK_SIZE / PAGE_SIZE;

    if (KSTACK_SIZE % PAGE_SIZE)
        pages++;

    MMU_free_pages((void *) addr, pages);
}
