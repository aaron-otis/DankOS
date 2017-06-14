#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#define DOUBLE_FAULT 0x8
#define GPF 0xD
#define PAGE_FAULT 0xE
#define SYSCALL_INT 0x80
#define PROC_EXIT_INT 0x83

#define SYSCALL_INT_ASM "int $0x80"
#define PROC_EXIT_INT_ASM "int $0x83"

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
