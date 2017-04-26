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

static struct IRQT interrupt_table[IDT_SIZE]; /* Table of interrupt functions */
ID IDT[IDT_SIZE]; /* Interrupt descriptor table. */
struct IDTR idtr;

extern void IRQ_end_of_interrupt(int irq) {

    PIC_sendEOI(irq);
}

static void unhandled_interrupt(int irq, int error, void *arg) {
    printk("Interrupt %d is not handled yet...\n", irq);
}

static void kb_interrupt_handler(int irq, int error, void *arg) {
    keypress kp;

    kp = KB_get_keypress(); /* Get key pressed. */
    IRQ_end_of_interrupt(irq); /* Signal EOI. */

    if (kp.codepoint)
        printk("%c", kp.codepoint); /* Print character. */
}

extern void IRQ_set_mask(int irq) {
}

extern void IRQ_clear_mask(int irq) {
}

extern int IRQ_get_mask(int IRQline) {
    int mask = 0;

    return mask;
}

extern void IRQ_handler(int irq, int error) {

    if (irq >= 0 && irq < IDT_SIZE)
        if (interrupt_table[irq].handler)
            interrupt_table[irq].handler(irq, error, interrupt_table[irq].arg);
}

extern void IRQ_init(void) {
    int i, debug = 1;

    /* Set up IDT table. */
    memset(IDT, 0, sizeof(ID) * IDT_SIZE); /* Zero IDT table. */
    idtr.length = IDT_SIZE * sizeof(ID); /* Store size of IDT table. */
    idtr.base = IDT; /* Store linear base address. */

    /* Populate idt table with descriptors. */
    populate_IDT_table();

    /* Use lidt asm instruction to set the IDT table. */
    __asm__("lidt %0" : : "m"(idtr));

    PIC_clear_mask(1);
    //while (debug); /* Infinite loop for debugging. */
    STI; /* Enable interrupts. */

    /* Populate interrupt_table with function pointers. */
    for (i = 0; i < IDT_SIZE; i++) {
        interrupt_table[i].handler = unhandled_interrupt;
        interrupt_table[i].arg = NULL;
    }


    /* Set keyboard interrupt handler. */
    interrupt_table[1].handler = kb_interrupt_handler;
    interrupt_table[0x21].handler = kb_interrupt_handler;
    asm("int $0x21"); /* Check if interrupts work! */
}
