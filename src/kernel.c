#include "kernel.h"
#include "kernel_tests.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "init.h"
#include "sh.h"
#include "gdt.h"

/* Global variables. */
segment_descriptor GDT[GDT_SIZE];
static TSS tss;
static struct segment_selector tss_sel;

static void halt_cpu() {

    for (;;) /* Infinite loop. */
        HALT_CPU
}

static void gdt_init() {

    /* Zero GDT. */
    memset(&GDT, 0, sizeof(segment_descriptor) * GDT_SIZE);

    /* Kernel Code Segment. */
    GDT[KERN_CS_INDEX].dpl = KERN_MODE;
    GDT[KERN_CS_INDEX].op_size = OP_SIZE_LEGACY;
    GDT[KERN_CS_INDEX].long_mode = LONG_MODE;
    GDT[KERN_CS_INDEX].present = PRESENT;
    GDT[KERN_CS_INDEX].code_data = CODE_SEG;
    GDT[KERN_CS_INDEX].conform_expand = CONFORMING;

    /* Kernel Data Segment. */
    GDT[KERN_DS_INDEX].present = PRESENT;

    /* TSS segment. */
    memset(&tss, 0, sizeof(tss));
    tss_sel.rpi = 0;
    tss_sel.ti = 0;
    
}

int kernel_main() {

    /* 
     * Initializations.
     */
    gdt_init();

    if (init() == EXIT_FAILURE)
        halt_cpu();

    /* Halt CPU at end for testing. */
    halt_cpu();

    return 0;
}
