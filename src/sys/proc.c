#include "proc.h"
#include "syscalls.h"
#include "kmalloc.h"
#include "memory.h"
#include "../drivers/interrupts.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../gdt.h"
#include <stddef.h>

#define RFLAG_INT_ENABELED (1 << 9)

static proc_t *main_proc_ptr;
static proc_t *process_list, *blocked_list;
static proc_t *process_list_tail, *blocked_list_tail;
static unsigned long long next_pid;
proc_t *cur_proc;
proc_t *next_proc;

static int schedule_proc(proc_t *proc) {
    proc_t *tail;
    int ret = EXIT_SUCCESS, ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    tail = process_list_tail;
    tail->next = proc;
    process_list_tail = proc;

    if (!process_list)
        process_list = process_list_tail;

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

    if (process_list) {
        next_proc = process_list;
        process_list = process_list->next;
    }
    else {
        next_proc = main_proc_ptr;
        next_proc->rflags = RFLAG_INT_ENABELED;
    }

    if (ints_enabled)
        STI;
}

void PROC_yield_isr(int irq, int err, void *arg) {

    PROC_reschedule();
}

void PROC_exit_isr(int irq, int err, void *arg) {
    proc_t *head = process_list, *prev;

    while (cur_proc && head != cur_proc) {
        prev = head;
        head = head->next;
    }

    if (cur_proc) /* Remove |cur_proc| from |process_list|. */
        prev->next = head->next;

    PROC_reschedule();
    MMU_free_kstack((void *) head->rsp);
    free(head);
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
    next_pid = 1; /* First pid. */

    /* Register yield system call. */
    SYSCALL_register_syscall(YIELD_SYSCALL, yield_syscall, NULL);

    /* Register ISR handler for yield and exit functions. */
    IRQ_set_handler(PROC_YIELD_INT, PROC_yield_isr, NULL);
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

    if (ints_enabled)
        STI;

    yield();
}

void exit(void) {
    __asm__ (PROC_EXIT_INT_ASM);
}

void PROC_create_kthread(kproc_t entry_point, void *arg) {
    proc_t *proc = kmalloc(sizeof(proc_t));
    proc_t *new;
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    memset(proc, 0, sizeof(proc_t)); /* Zero all fields. */
    proc->rip = (uint64_t) entry_point;
    proc->cs = KERN_CS_OFFSET;
    proc->rsp = MMU_alloc_kstack();
    proc->ss = 0;
    proc->rflags = RFLAG_INT_ENABELED;
    proc->rdi = (uint64_t) arg;

    /* 
     * Put call to |exit| at bottom of stack so that the process will exit by 
     * default.
     */
    *proc->rsp = (uint64_t) exit;
    proc->rsp--; /* Decrement stack pointer. */

    /* Add to process queue. */
    schedule_proc(proc);

    if (ints_enabled)
        STI;
}
