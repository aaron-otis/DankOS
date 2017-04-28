#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "../lib/stdint.h"

#define IDT_SIZE 256
#define TSS_TYPE 0x9
#define TSS_GRANULARITY 0
#define PAGE_SIZE 4096

extern void IRQ_init(void);
extern int IRQ_get_mask(int IRQline);
extern void IRQ_end_of_interrupt(int irq);

typedef void (*irq_handler_t)(int, int, void *);
extern int IRQ_set_handler(int irq, irq_handler_t handler, void *arg);

static inline int are_interrupts_enabled() {
    unsigned long flags;

    asm volatile ( "pushf\n\t" "pop %0" : "=g"(flags) );

    return flags & (1 << 9);
}

typedef struct {
    uint16_t offset1;
    uint16_t selector; /* Should be selector for kernel's code segment. */
    uint16_t ist:3;
    uint16_t reserved1:5;
    uint16_t type:4; /* Type of interrupt. Should be 0xE for interrupt gate. */
    uint16_t zero:1;
    uint16_t dpl:2; /* Protection level. */
    uint16_t present:1; /* Indicates a valid table entry. Should be 1. */
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved2;
} __attribute__ ((packed)) ID; /* Interrupt descriptor. */

typedef struct {
    uint16_t limit;
    uint16_t base1;
    uint16_t base2:8;
    uint16_t type:4;
    uint16_t zero:1;
    uint16_t dpl:2;
    uint16_t present:1;
    uint16_t segment_limit:4;
    uint16_t avl:1;
    uint16_t reserved1:2;
    uint16_t granularity:1;
    uint16_t base3:8;
    uint32_t reserved2;
    uint32_t reserved3:8;
    uint32_t legacy:5;
    uint32_t reserved4:19;
} __attribute__ ((packed)) TD; /* TSS descriptor. */

extern ID IDT[IDT_SIZE]; /* Interrupt descriptor table. */
extern struct IDTR idtr;

#define CLI __asm__("cli")
#define STI __asm__("sti")

#endif
