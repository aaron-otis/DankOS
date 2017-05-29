#ifndef _MEMORY_H
#define _MEMORY_H

#include "../lib/stdint.h"

/* Virtual Address Space Layout:
 *
 * Identity map         0x0
 * Kernel stacks        0x10000000000
 * Reserved/Growth      0x20000000000
 * Kernel heap          0xF0000000000
 * User space           0x100000000000
 */

#define PAGE_SIZE 4096
#define KSTACKS_ADDR 0x10000000000ULL
#define KRESERVED_ADDR 0x20000000000ULL
#define KHEAP_ADDR 0xF0000000000ULL

#define KSTACK_SIZE 0x200000ULL
#define KHEAP_SIZE 0x200000ULL
#define USR_STACK_SIZE 0x100000ULL
#define USR_HEAP_SIZE 0x100000ULL

typedef struct {
    void *address;
    long length;
} MEM_phys_block;

/** @brief Node for the free list of page frames.
 */
typedef struct page_frame {
    struct page_frame *next;
} page_frame;

/** @brief Structure of CR3 Register.
 *
 * Bit 3 is the Page-Level Writethrough: 0 denotes a writeback caching policy, 
 * while 1 denote a writethrough caching policy.
 * Bit 4 is the Page-Level Cache Disabled: 0 denotes cacheable, 1 denotes not 
 * cacheable.
 * In long mode, a 40-bit base address is used.
 */
typedef struct cr3_register {
    uint64_t reserved1:3;
    uint64_t pwt:1;
    uint64_t pcd:1;
    uint64_t reserved2:7;
    uint64_t base_addr:40;
    uint64_t reserved3:12;
} __attribute__((packed)) CR3;

typedef struct page_map_level_4 {
    uint64_t present:1;
    uint64_t r_w:1; /* Read/write bit. */
    uint64_t u_s:1; /* User/supervisor mode. Set to 1 for user mode. */
    uint64_t pwt:1; /* Page-Level Writethrough. */
    uint64_t pcd:1; /* Page Caching Disabled. */
    uint64_t a:1;   /* Accessed bit. */
    uint64_t ign:1;
    uint64_t mbz:2;
    uint64_t avl:3; /* Available to software. */
    uint64_t base_addr:51; /* 40 bit base address combined with 11 bit avail. */
    uint64_t nx:1;
} __attribute__((packed)) PML4;

typedef struct page_directory_pointer_table {
    uint64_t present:1;
    uint64_t r_w:1;
    uint64_t u_s:1;
    uint64_t pwt:1;
    uint64_t pcd:1;
    uint64_t a:1;
    uint64_t ign:1;
    uint64_t zero:1;
    uint64_t mbz:1;
    uint64_t avl:3;
    uint64_t base_addr:51;
    uint64_t nx:1;
} __attribute__((packed)) PDP;

typedef struct page_directory_pointer_table PD;

typedef struct page_table {
    uint64_t present:1;
    uint64_t r_w:1;
    uint64_t u_s:1;
    uint64_t pwt:1;
    uint64_t pcd:1;
    uint64_t a:1;
    uint64_t d:1;           /* Dirty bit. */
    uint64_t pat:1;         /* Page-Attribute Table. */
    uint64_t global_page:1;
    uint64_t avl:3;
    uint64_t base_addr:51;
    uint64_t nx:1;
} __attribute__((packed)) PT;

/* Page frame allocator. */
void MMU_pf_init();
void *MMU_pf_alloc(void);
int MMU_pf_free(void *pf);

/* Virtual page allocator. */
int MMU_init();
void *MMU_alloc_page();
void *MMU_alloc_pages(unsigned int num);
void MMU_free_page(void *);
void MMU_free_pages(void *, unsigned int num);
extern void MMU_page_fault_handler(int irq, int error, void *arg);
void *kbrk(intptr_t increment);

#endif
