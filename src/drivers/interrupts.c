#include "interrupts.h"
#include "interrupt_externs.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "pic.h"
#include "keyboard.h"

struct IRQT {
    void *arg;
    irq_handler_t handler;
};

struct IDTR {
    uint16_t length;
    void *base;
} __attribute__((packed));

struct TSS_selector {
    uint16_t rpi:2; /* Privilege level (CPL). */
    uint16_t ti:1; /* Must be 0 to indicate the GDT is used. */
    uint16_t index:13;
};

struct TSS {
    uint32_t reserved1;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t base;
};

/* Table of interrupt functions */
static struct IRQT interrupt_table[IDT_SIZE];
ID IDT[IDT_SIZE]; /* Interrupt descriptor table. */
struct IDTR idtr;
static struct TSS tss;
static struct TSS_selector tss_sel;

extern void IRQ_end_of_interrupt(int irq) {

    PIC_sendEOI(irq);
}

static void unhandled_interrupt(int irq, int error, void *arg) {
    printk("Interrupt %d is not handled yet...\n", irq);
}

extern int IRQ_get_mask(int IRQline) {
    int mask = 0;

    return mask;
}

extern void IRQ_handler(int irq, int error) {

    if (irq >= 0 && irq < IDT_SIZE) /* Ensure IRQ is within bounds. */
        if (interrupt_table[irq].handler) /* Check for null pointer. */
            interrupt_table[irq].handler(irq, error, interrupt_table[irq].arg);

    IRQ_end_of_interrupt(irq); /* Signal EOI. */
}

extern void IRQ_init(void) {
    int i;

    /* Set up IDT table. */
    memset(IDT, 0, sizeof(ID) * IDT_SIZE); /* Zero IDT table. */
    idtr.length = IDT_SIZE * sizeof(ID); /* Store size of IDT table. */
    idtr.base = IDT; /* Store linear base address. */

    /* Populate idt table with descriptors. */
    populate_IDT_table();

    /* Use lidt asm instruction to set the IDT table. */
    __asm__("lidt %0" : : "m"(idtr));

    /* Populate interrupt_table with pointers to unhandled_interrupt. */
    for (i = 0; i < IDT_SIZE; i++) {
        interrupt_table[i].handler = unhandled_interrupt;
        interrupt_table[i].arg = NULL;
    }

    /* Set up TSS. */
    memset(&tss, 0, sizeof(tss));
    tss_sel.rpi = 0;
    tss_sel.ti = 0;
    
}

extern int IRQ_set_handler(int irq, irq_handler_t handler, void *arg) {
    int ret = EXIT_FAILURE;

    if (irq >= 0 && irq < IDT_SIZE) {
        interrupt_table[irq].handler = handler;
        interrupt_table[irq].arg = arg;
        ret = EXIT_SUCCESS;
    }

    return ret;
}
