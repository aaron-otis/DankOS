#include "syscalls.h"
#include "../drivers/interrupts.h"
#include "../lib/string.h"
#include <stddef.h>

static Syscall syscall_table[NUM_SYSCALLS];
static int syscall_num;

static int SYSCALL_call_syscall(int irq, int err, void * arg, int sys) {

    syscall_table[sys].syscall(syscall_table[sys].arg);
}

void SYSCALL_generic_isr(int irq, int err, void *arg) {

    /*__asm__ ("call SYSCALL_call_syscall");*/
    syscall_table[syscall_num].syscall(syscall_table[syscall_num].arg);
}

int SYSCALL_register_syscall(int num, syscall_handler syscall, void *arg) {
    int ret = -1, ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    if (num >= 0 && num <= NUM_SYSCALLS) {
        syscall_table[num].syscall = syscall;
        syscall_table[num].arg = arg;
        ret = 0;
    }

    if (ints_enabled)
        STI;

    return ret;
}

void SYSCALL_init(void) {
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    memset(syscall_table, 0, sizeof(Syscall) * NUM_SYSCALLS);
    IRQ_set_handler(SYSCALL_INT, SYSCALL_generic_isr, NULL);

    if (ints_enabled)
        STI;
}

void SYSCALL_generic_syscall(int u1, int u2, void *u3, int syscall) {
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    syscall_num = syscall;

    if (ints_enabled)
        STI;

    __asm__ (SYSCALL_INT_ASM);
}
