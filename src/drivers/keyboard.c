#include "keyboard.h"
#include "interrupts.h"
#include "pic.h"

#define KB_TIMEOUT 3
#define KB_DEFAULT_SCAN_CODE KB_SCAN_CODE_2
#define KB_QUEUE_FULL 2
#define KB_QUEUE_EMPTY 3
#define QI_READ 0
#define QI_WRITE 1

#define LAST_KEY 0x46
#define KB_MASK 1

/* Global variables. */

/*
static struct {
    uint8_t operation; 
    uint8_t data; 
    uint8_t expected_res; 
} queue_item;
*/

struct KB_int_arg {
    key_handler_t handler;
};

//static uint8_t cmd_queue[KB_QUEUE_SIZE];
//static uint8_t *queue_head;
//static uint8_t *queue_tail;
static int modifiers_pressed;
static int toggles_set;
static struct KB_int_arg arg;

/* Static functions. */

/*
static void reset_queue() {
    queue_tail = NULL;
    queue_head = cmd_queue;
    memset(cmd_queue, 0, KB_QUEUE_SIZE);
}

static int queue(uint8_t item) {
    uint8_t *shift;
    int i;

    if (!queue_tail) 
        if (queue_head != cmd_queue) 
            return EXIT_FAILURE;
        else 
            queue_tail = queue_head; 

    if (queue_tail == cmd_queue + KB_QUEUE_SIZE) {
        if (queue_head == cmd_queue) 
            return KB_QUEUE_FULL;
        else { 
            shift = queue_head;

            for (i = 0; shift <= queue_tail; i++)
                cmd_queue[i] = *shift++;

            queue_head = cmd_queue; 
            queue_tail -= --i; 
        }
    }

    *queue_tail++ = item;

    return EXIT_SUCCESS;
}

static int dequeue(uint8_t *item) {

    if (queue_head == cmd_queue + KB_QUEUE_SIZE) {
        reset_queue();
        return KB_QUEUE_EMPTY;
    }
    else if (queue_head > queue_tail) {
        reset_queue();
        return KB_QUEUE_EMPTY;
    }

    *item = *queue_head++;
    return EXIT_SUCCESS;
}

static uint8_t resend_cmd(uint8_t code) {
    int i, res, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    i = 0; 

    while ((res = PS2_polling_read()) == KB_RESEND_CMD && i++ < KB_TIMEOUT)
        PS2_write(code);

    if (int_enabled)
        STI;

    return res;
}

static int exec_cmds() {
    uint8_t cmd;
    int res;

    while (dequeue(&cmd) == EXIT_SUCCESS) {
        res = PS2_write(cmd);
    }

    res = PS2_polling_read(); 
    if (res == KB_RESEND_CMD) { 
        res = resend_cmd(cmd); 

        if (res == KB_RESEND_CMD) 
            return KB_TIMEOUT_EXCEEDED;
    }

    return res;
}
*/

/* External functions. */

extern int KB_reset(){
    int res, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    PS2_write(KB_RESET_TEST);
    res = PS2_polling_read(); /* Need to use polling read! */

    if (res == KB_RESEND_CMD) {
        PS2_write(KB_RESET_TEST);
        res = PS2_polling_read();
    }

    if (res == KB_CMD_ACK) { /* Check that command was acknowledged. */
        res = PS2_polling_read();
    }
    else { /* Otherwise bail out. */
        if (int_enabled)
            STI;
        return res;
    }

    if (res == KB_PASSED_SELF_TEST)
        res = EXIT_SUCCESS;

    if (int_enabled)
        STI;

    return res;
}

extern int KB_enable(){
    int res, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    PS2_write(KB_ENABLE_SCANNING);
    res = PS2_polling_read();

    if (res == KB_CMD_ACK)
        res = EXIT_SUCCESS;

    if (int_enabled)
        STI;
    return res;
}

/*
extern int KB_disable(){
    int res, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    queue(KB_DISABLE_SCANNING); 
    res = exec_cmds();

    if (res == KB_CMD_ACK)
        res = EXIT_SUCCESS;

    if (int_enabled)
        STI;
    return res;
}
*/

extern int KB_set_scan_code(uint8_t code){
    int res, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    /* Check for valid scan codes. */
    if (code > KB_SCAN_CODE_3 || code < KB_SCAN_CODE_1) {
        if (int_enabled)
            STI;
        return KB_INVALID_SCAN_CODE;
    }

    PS2_write(KB_GET_SET_SCAN_CODE);
    res = PS2_polling_read();
    PS2_write(code);
    res = PS2_polling_read();

    if (res == KB_CMD_ACK) /* Keyboard successfully received code. */
        res = EXIT_SUCCESS;
    else
        res = EXIT_FAILURE;

    if (int_enabled)
        STI;

    return res;
}

extern uint8_t KB_get_scan_code(){
    int res, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    PS2_write(KB_GET_SET_SCAN_CODE);
    res = PS2_polling_read();

    if (res == KB_CMD_ACK) /* Keyboard successfully received code. */
        res= PS2_polling_read(); /* Return scan code. */

    if (int_enabled)
        STI;
    return res;
}

extern int KB_init(){
    int res, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    arg.handler = NULL;
    modifiers_pressed = 0;
    toggles_set = 0;


    /* Reset keyboard. */
    res = KB_reset();
    if (res != EXIT_SUCCESS) {
        if (int_enabled)
            STI;
        return res;
    }

    /* Set keyboard to a known scan code. */
    res = KB_set_scan_code(KB_DEFAULT_SCAN_CODE); /* Default scan code. */
    if (res != EXIT_SUCCESS) {
        if (int_enabled)
            STI;
        return res;
    }

    /* 
     * Will need to check what scan code set is actually being used since
     * keyboard may not support the code set we set. 
     */

    /* Enable keyboard. */
    res = KB_enable();
    if (res != EXIT_SUCCESS) {
        if (int_enabled)
            STI;
        return res;
    }

    PIC_clear_mask(KB_MASK); /* Clear mask for keyboard. */

    /* Set interrupt handler. */
    IRQ_set_handler(KB_MASK + PIC_MASTER_OFFSET, KB_interrupt_handler, &arg);

    if (int_enabled)
        STI;

    return EXIT_SUCCESS;
}

extern keypress KB_get_keypress() {
    keypress key;
    uint8_t res;

    key.codepoint = 0; /* Zero this in case characters are not printable. */
    res = PS2_read(); /* Use interrupt driven function. */


    /* Check to see if key is released or pressed. */
    if (res - KB_KEY_RELEASE_OFFSET >= 0)
        key.pressed = 0;
    else
        key.pressed = 1;


    if (res <= KB_KEY_RELEASE_OFFSET) {
        /* Check for modifiers. */
        if (KB_code_set_1[res] == KEY_L_SHIFT)
            modifiers_pressed |= L_SHIFT_PRESSED;
        else if (KB_code_set_1[res] == KEY_R_SHIFT)
            modifiers_pressed |= R_SHIFT_PRESSED;
        else if (KB_code_set_1[res] == KEY_L_CTRL)
            modifiers_pressed |= L_CTL_PRESSED;
        else if (KB_code_set_1[res] == KEY_R_CTRL)
            modifiers_pressed |= R_CTL_PRESSED;
        else if (KB_code_set_1[res] == KEY_L_ALT)
            modifiers_pressed |= L_ALT_PRESSED;
        else if (KB_code_set_1[res] == KEY_R_ALT)
            modifiers_pressed |= R_ALT_PRESSED;
        /* Check for toggles. */
        else if (KB_code_set_1[res] == KEY_CAPS_LOCK)
            toggles_set |= CAPS_LOCK_ON;
        else if (KB_code_set_1[res] == KEY_NUM_LOCK)
            toggles_set |= NUM_LOCK_ON;
        else if (KB_code_set_1[res] == KEY_SCROLL_LOCK)
            toggles_set |= SCROLL_LOCK_ON;
    }
    else {
        res -= KB_KEY_RELEASE_OFFSET; /* Remove release offset. */

        /* Check for modifiers. */
        if (KB_code_set_1[res] == KEY_L_SHIFT)
            modifiers_pressed &= ~L_SHIFT_PRESSED;
        else if (KB_code_set_1[res] == KEY_R_SHIFT)
            modifiers_pressed &= ~R_SHIFT_PRESSED;
        else if (KB_code_set_1[res] == KEY_L_CTRL)
            modifiers_pressed &= ~L_CTL_PRESSED;
        else if (KB_code_set_1[res] == KEY_R_CTRL)
            modifiers_pressed &= ~R_CTL_PRESSED;
        else if (KB_code_set_1[res] == KEY_L_ALT)
            modifiers_pressed &= ~L_ALT_PRESSED;
        else if (KB_code_set_1[res] == KEY_R_ALT)
            modifiers_pressed &= ~R_ALT_PRESSED;
        /* Check for toggles. */
        else if (KB_code_set_1[res] == KEY_CAPS_LOCK)
            toggles_set &= ~CAPS_LOCK_ON;
        else if (KB_code_set_1[res] == KEY_NUM_LOCK)
            toggles_set &= ~NUM_LOCK_ON;
        else if (KB_code_set_1[res] == KEY_SCROLL_LOCK)
            toggles_set &= ~SCROLL_LOCK_ON;

    }

    /* Check if key pressed was a modifier. */
    if (KB_code_set_1[res] >= KEY_L_SHIFT && KB_code_set_1[res] <= 
     KEY_SCROLL_LOCK)
        key.codepoint = 0;
    else if (key.pressed) /* Otherwise it is a printable character. */
        key.codepoint = KB_code_set_1[res];
    else
        key.codepoint = 0;

    key.modifiers = modifiers_pressed;
    key.toggles = toggles_set;

    return key;
}

/*
extern int KB_set_default_params(){
}

extern uint8_t KB_resend(){
}

extern int KB_set_lights(uint8_t code){
}
extern int KB_set_rate(uint8_t rate){
}
extern int KB_get_rate(){
}
extern int KB_set_delay(uint8_t delay){
}
extern int KB_get_delay(){
}
extern int KB_set_all_keys_type(int type){
}
extern int KB_set_key_type(int scan_code, int type){
}
*/

extern void KB_interrupt_handler(int irq, int error, void *arg) {
    keypress kp;

    kp = KB_get_keypress(); /* Get key pressed. */

    if (kp.codepoint)
        printk("%c", kp.codepoint); /* Print character. */
}
