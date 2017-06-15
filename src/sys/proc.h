#ifndef _PROC_H
#define _PROC_H

#include "../lib/stdint.h"

typedef void (*kproc_t)(void*);
typedef uint64_t pid_t;

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
    pid_t pid;
    /* file descriptors. */
    struct proc_t *proc_next;
    struct proc_t *sched_next;
    struct proc_t *block_next;
    struct proc_t *proc_prev;
    struct proc_t *sched_prev;
    struct proc_t *block_prev;
} __attribute__ ((packed)) proc_t;

typedef enum {PROCESS, SCHEDULE, BLOCK} Queue;

struct process_queue {
    proc_t *head;
    Queue queue_type;
};

typedef struct process_queue * ProcessQueue;

extern proc_t *cur_proc;
extern proc_t *next_proc;

void PROC_init(void);
void PROC_run(void);
proc_t *PROC_create_kthread(kproc_t entry_point, void *arg);
void PROC_reschedule(void);
void kexit(void);
void yield(void);

void PROC_block_on(ProcessQueue queue, int enable_ints);
void PROC_unblock_all(ProcessQueue queue);
void PROC_unblock_head(ProcessQueue queue);
void PROC_init_queue(ProcessQueue queue, Queue type);

#endif
