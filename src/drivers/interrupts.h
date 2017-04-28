#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

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


#define CLI __asm__("cli")
#define STI __asm__("sti")

#endif
