#ifndef _PROC_H
#define _PROC_H

#include "../lib/stdint.h"

typedef void (*kproc_t)(void*);
typedef uint64_t pid_t;

void PROC_init(void);
void PROC_run(void);
void PROC_create_kthread(kproc_t entry_point, void *arg);
void PROC_reschedule(void);
void exit(void);
void yield(void);

typedef struct proc_t {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rbp;
    uint64_t *rsp;
    uint64_t rip;
    uint64_t rflags;
    uint64_t cs;
    uint64_t ss;
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
    pid_t id;
    /* file descriptors. */
    struct proc_t *next;
} __attribute__ ((packed)) proc_t;

extern proc_t *cur_proc;
extern proc_t *next_proc;

#endif
