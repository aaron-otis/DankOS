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
static ProcessQueue process_list, sched_list;
static proc_t *process_list_tail, *sched_list_tail;
static unsigned long long next_pid;
proc_t *cur_proc;
proc_t *next_proc;

static void unlink_proc(proc_t *proc, ProcessQueue queue) {
    proc_t *temp = queue->head;

    if (queue->head == proc) {
        if (queue->queue_type == PROCESS) {
            queue->head = proc->proc_next;
        }
        else if (queue->queue_type == SCHEDULE) {
            queue->head = proc->sched_next;
        }
        else if (queue->queue_type == BLOCK) {
            queue->head = proc->block_next;
        }
        else {
            printk("\nunlink_proc error. Invalid queue type.\n");
            return;
        }
    }
    else if (queue->head) {
        if (queue->queue_type == PROCESS) {
            while (temp != proc)
                temp = temp->proc_next;

            if (temp)
                temp->proc_prev->proc_next = temp->proc_next;
            else
                printk("\nUnable to unlink. Process not found!\n");
        }
        else if (queue->queue_type == SCHEDULE) {
            while (temp != proc)
                temp = temp->sched_next;

            if (temp)
                temp->sched_prev->sched_next = temp->sched_next;
            else
                printk("\nUnable to unlink. Process not found!\n");
        }
        else if (queue->queue_type == BLOCK) {
            while (temp != proc)
                temp = temp->block_next;

            if (temp)
                temp->block_prev->block_next = temp->block_next;
            else
                printk("\nUnable to unlink. Process not found!\n");
        }
        else {
            printk("\nunlink_proc error. Invalid queue type.\n");
            return;
        }
    }


}

static void append_proc(proc_t *proc, ProcessQueue queue) {
    proc_t *temp = queue->head;

    proc->proc_next = proc->sched_next = proc->block_next = NULL;
    if (temp) {
        if (queue->queue_type == PROCESS) {
            while (temp->proc_next)
                temp = temp->proc_next;

            temp->proc_next = proc;
        }
        else if (queue->queue_type == SCHEDULE) {
            while (temp->sched_next)
                temp = temp->sched_next;

            temp->sched_next = proc;
            proc->sched_prev = temp;
            proc->block_prev = NULL;
        }
        else if (queue->queue_type == BLOCK) {
            while (temp->block_next)
                temp = temp->block_next;

            temp->block_next = proc;
            proc->block_prev = temp;
            proc->sched_prev = NULL;
        }
        else {
            printk("unlink_proc error. Invalid queue type.\n");
            return;
        }
    }
    else {
        queue->head = proc;
        if (queue->queue_type == PROCESS) {
            queue->head->proc_prev = NULL;
        }
        else if (queue->queue_type == SCHEDULE) {
            queue->head->sched_prev = NULL;
            queue->head->block_prev = NULL;
        }
        else if (queue->queue_type == BLOCK) {
            queue->head->block_prev = NULL;
            queue->head->sched_prev = NULL;
        }
        else {
            printk("unlink_proc error. Invalid queue type.\n");
            return;
        }
    }
}

static int process_tracked(proc_t *proc) {
    proc_t *temp = process_list->head;
    int ret = EXIT_FAILURE;

    if (proc) {
        while (temp && temp != proc)
            temp = temp->proc_next;

        if (temp == proc)
            ret = EXIT_SUCCESS;
    }

    return ret;
}

static int schedule_proc(proc_t *proc) {
    proc_t *tail;
    int ret = EXIT_SUCCESS, ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    append_proc(proc, sched_list);
    if (!process_tracked(proc))
        append_proc(proc, process_list);

    if (ints_enabled)
        STI;

    return ret;
}

void PROC_reschedule(void) {
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    if (!cur_proc)
        printk("PROC_reschedule error. cur_proc is null!\n");

    next_proc = cur_proc->sched_next;
    if (!next_proc) {
        if (sched_list->head)
            next_proc = sched_list->head;
        else {
            next_proc = main_proc_ptr;
            next_proc->rflags = RFLAG_INT_ENABELED;
        }
    }

    if (ints_enabled)
        STI;
}

void PROC_yield_isr(int irq, int err, void *arg) {

    PROC_reschedule();
}

void PROC_exit_isr(int irq, int err, void *arg) {

    unlink_proc(cur_proc, sched_list);
    unlink_proc(cur_proc, process_list);

    PROC_reschedule();
    MMU_free_kstack((void *) cur_proc->rsp);
    kfree(cur_proc);
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

    /* Initialize queues. */
    process_list = kmalloc(sizeof( ProcessQueue));
    sched_list = kmalloc(sizeof( ProcessQueue));
    PROC_init_queue(process_list, PROCESS);
    PROC_init_queue(sched_list, SCHEDULE);

    process_list_tail = sched_list_tail = NULL;
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

void PROC_block_on(ProcessQueue queue, int enable_ints) {

    if (!queue)
        return;

    unlink_proc(cur_proc, sched_list);
    append_proc(cur_proc, queue);

    if (enable_ints)
        STI;

    yield();
}

void PROC_unblock_head(ProcessQueue queue) {
    proc_t *temp = queue->head;

    unlink_proc(temp, queue);
    append_proc(temp, sched_list);
}

void PROC_unblock_all(ProcessQueue queue) {

    while (queue->head)
        PROC_unblock_head(queue);
}

void PROC_init_queue(ProcessQueue queue, Queue type) {
    queue->head = NULL;
    queue->queue_type = type;
}
