#ifndef _PROC_H
#define _PROC_H

typedef void (*kproc_t)(void*);

void PROC_run(void);
void PROC_create_kthread(kproc_t entry_point, void *arg);
void PROC_reschedule(void);
void kexit(void);

static inline void yield(void) {
          asm volatile ( "INT $123" );
}

typedef struct proc_t {
} proc_t;

#endif
