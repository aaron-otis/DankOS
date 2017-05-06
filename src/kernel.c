#include "kernel.h"
#include "kernel_tests.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "drivers/interrupts.h"
#include "sys/multiboot.h"
#include "sys/memory.h"
#include "init.h"
#include "gdt.h"

/* Global variables. */
static TSS tss;
uint8_t IST_stacks[NUM_IST_STACKS][IST_STACK_SIZE];

static void halt_cpu() {

    for (;;) /* Infinite loop. */
        HALT_CPU
}

static void tss_init() {
    uint64_t tss_addr = (uint64_t) &tss, tss_index = TSS_INDEX;
    segment_descriptor *gdt = (segment_descriptor *) &gdt64;
    struct segment_selector tss_sel;
    int int_enabled = 0;
    TD tss_desc;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    memset(&tss, 0, sizeof(tss)); /* Zero TSS. */

    /* Set up TSS descriptor. */
    tss_desc.limit1 = sizeof(tss);
    tss_desc.base1 = tss_addr & OFF_1_MASK;
    tss_desc.base2 = (tss_addr >> OFF_2_SHIFT) & TSS_OFF_2_MASk;
    tss_desc.type = TSS_TYPE;
    tss_desc.dpl = KERN_MODE;
    tss_desc.present = PRESENT;
    tss_desc.base3 = (tss_addr >> TSS_OFF_3_SHIFT) & TSS_OFF_2_MASk;
    tss_desc.base4 = (tss_addr >> OFF_3_SHIFT);

    /* Copy TSS descriptor into the proper place in the GDT. */
    memcpy(&gdt[TSS_INDEX], &tss_desc, sizeof(tss_desc));

    /* Set up TSS selector. */
    tss_sel.rpi = 0;
    tss_sel.ti = 0;
    tss_sel.index = tss_index;
    __asm__("ltr %0": : "m"(tss_sel)); /* Load TSS selector. */

    /* Set up critical ISTs. */
    tss.ist1 = (uint64_t) (&IST_stacks[1]);
    tss.ist2 = (uint64_t) (&IST_stacks[2]);
    tss.ist3 = (uint64_t) (&IST_stacks[3]);

    if (int_enabled)
        STI;
}

int kernel_main(MB_basic_tag *mb_tag) {

    /* 
     * Initializations.
     */

    tss_init(); /* Initialize TSS. */

    if (init() == EXIT_FAILURE)
        halt_cpu();

    MMU_pf_init(mb_tag);

    /* Halt CPU at end for testing. */
    halt_cpu();

    return 0;
}
