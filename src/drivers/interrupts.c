#include "interrupts.h"
#include "interrupt_externs.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../gdt.h"
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

/* Table of interrupt functions */
static struct IRQT interrupt_table[IDT_SIZE];
ID IDT[IDT_SIZE]; /* Interrupt descriptor table. */
struct IDTR idtr;

extern void IRQ_end_of_interrupt(int irq) {

    PIC_sendEOI(irq);
}

static void unhandled_interrupt(int irq, int error, void *arg) {
    register intptr_t sp asm ("rsp");

    printk("\nInterrupt %d is not handled yet... Error: %d\n", irq, error);
    IRQ_end_of_interrupt(irq);
}

extern int IRQ_get_mask(int IRQline) {
    int mask = 0;

    return mask;
}

extern void IRQ_handler(int irq, int error) {

    if (irq >= 0 && irq < IDT_SIZE) /* Ensure IRQ is within bounds. */
        if (interrupt_table[irq].handler) /* Check for null pointer. */
            interrupt_table[irq].handler(irq, error, interrupt_table[irq].arg);
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

    /* Modify critical interrupts to have different stacks. */
    IDT[DOUBLE_FAULT].ist = 1;
    IDT[GPF].ist = 2;
    IDT[PAGE_FAULT].ist = 3;
    IDT[PROC_EXIT_INT].ist = 4;
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
