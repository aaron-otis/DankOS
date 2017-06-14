#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#define NUM_SYSCALLS 256

/* System call numbers. */
#define YIELD_SYSCALL 0x7B /* System call for yield. */

typedef void (*syscall_handler)(void *);

typedef struct Syscall {
    syscall_handler syscall;
    void *arg;
} Syscall;

void SYSCALL_init(void);
void SYSCALL_generic_syscall(int u1, int u2, void *u3, int syscall);
int SYSCALL_register_syscall(int num, syscall_handler syscall, void *arg);

#endif
