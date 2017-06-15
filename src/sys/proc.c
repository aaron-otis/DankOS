#include "proc.h"
#include "syscalls.h"
#include "kmalloc.h"
#include "memory.h"
#include "../drivers/interrupts.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../lib/debug.h"
#include "../gdt.h"
#include <stddef.h>

#define RFLAG_INT_ENABELED (1 << 9)

static proc_t *main_proc_ptr;
static proc_t *process_list, *blocked_list;
static proc_t *process_list_tail, *blocked_list_tail;
static unsigned long long next_pid;
proc_t *cur_proc;
proc_t *next_proc;

static void unlink_proc(proc_t proc, ProcessQueue queue) {
}

static void append_proc(proc_t proc, ProcessQueue queue) {
}

static int schedule_proc(proc_t *proc) {
    proc_t *tail;
    int ret = EXIT_SUCCESS, ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    if (process_list_tail) {
        tail = process_list_tail;
        tail->proc_next = proc;
    }

    process_list_tail = proc;
    if (!process_list)
        process_list = process_list_tail;

    process_list_tail->proc_next = process_list;

    if (ints_enabled)
        STI;

    return ret;
}

static int block_proc(proc_t *proc) {

    return EXIT_SUCCESS;
}

void PROC_reschedule(void) {
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    if (!process_list) {
        next_proc = main_proc_ptr;
        next_proc->rflags = RFLAG_INT_ENABELED;
    }
    else if (cur_proc->proc_next)
        next_proc = cur_proc->proc_next;
    else if (process_list)
        next_proc = process_list;

    if (ints_enabled)
        STI;
}

void PROC_yield_isr(int irq, int err, void *arg) {

    PROC_reschedule();
}

void PROC_exit_isr(int irq, int err, void *arg) {
    proc_t *head = process_list, *prev = NULL;

    while (cur_proc && head != cur_proc) {
        prev = head;
        head = head->proc_next;
    }

    /* If there is only one process in the list, set the list to NULL. */
    if (process_list == process_list_tail)
        process_list = NULL;
    else if (head == process_list) { /* Removed the first item in the list. */
        process_list = process_list->proc_next;
        process_list_tail->proc_next = process_list;
    }
    else if (head) /* Remove |cur_proc| from |process_list|. */
        prev->proc_next = head->proc_next;
    else {
        printk("PROC_exit_isr error()\n");
        HALT_CPU
    }

    PROC_reschedule();
    MMU_free_kstack((void *) head->rsp);
    kfree(head);
    cur_proc = NULL;
}

void yield_syscall(void *arg) {
    PROC_reschedule();
}

void yield(void) {
    SYSCALL_generic_syscall(0, 0, 0, YIELD_SYSCALL);
}

void PROC_init(void) {
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    cur_proc = next_proc = NULL; /* No processes scheduled yet. */
    main_proc_ptr = NULL; /* Main process struct not created yet. */
    process_list = blocked_list = NULL; /* No threads created yet. */
    process_list_tail = blocked_list_tail = NULL;
    next_pid = 1; /* First pid. */

    /* Register yield system call. */
    SYSCALL_register_syscall(YIELD_SYSCALL, yield_syscall, NULL);

    /* Register ISR handler for yield and exit functions. */
    IRQ_set_handler(PROC_EXIT_INT, PROC_exit_isr, NULL);

    if (ints_enabled)
        STI;
}

void PROC_run(void) {
    proc_t main_proc;
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    memset(&main_proc, 0, sizeof(main_proc)); /* Zero |main_proc|. */
    main_proc_ptr = &main_proc;
    main_proc.cs = KERN_CS_OFFSET;
    cur_proc = &main_proc;

    /* Set the next thread to be run from the queue of scheduled processes. */
    cur_proc->proc_next = process_list;

    if (ints_enabled)
        STI;

    yield();
}

void kexit(void) {
    __asm__ (PROC_EXIT_INT_ASM);
}

proc_t *PROC_create_kthread(kproc_t entry_point, void *arg) {
    proc_t *proc, *new, *ret = NULL;
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    proc = kmalloc(sizeof(proc_t));
    if (proc == (void *) -1)
        printk("kmalloc error in PROC_create_kthread\n");
    else {
        memset(proc, 0, sizeof(proc_t)); /* Zero all fields. */
        proc->rip = (uint64_t) entry_point;
        proc->cs = KERN_CS_OFFSET;
        proc->rsp = MMU_alloc_kstack();

        if (proc->rsp > 0) {
            proc->ss = 0;
            proc->rflags = RFLAG_INT_ENABELED;
            proc->rdi = (uint64_t) arg;

            /* 
             * Put call to |exit| at bottom of stack so that the process will 
             * exit by default.
             */
            *proc->rsp = (uint64_t) kexit;
            proc->rsp--; /* Decrement stack pointer. */

            /* Add to process queue. */
            schedule_proc(proc);
            proc->pid = next_pid++;
            ret = proc;
        }
        else
            printk("MMU_alloc_kstack error\n");
    }

    if (ints_enabled)
        STI;

    return ret;
}

void PROC_block_on(ProcessQueue *pq, int enable_ints) {
}

void PROC_unblock_all(ProcessQueue *pg) {
}

void PROC_unblock_head(ProcessQueue *pg) {
}

void PROC_init_queue(ProcessQueue *pg) {
}
