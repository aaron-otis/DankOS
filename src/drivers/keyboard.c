#include "keyboard.h"

#define KB_TIMEOUT 3
#define KB_DEFAULT_SCAN_CODE KB_SCAN_CODE_2
#define KB_QUEUE_FULL 2
#define KB_QUEUE_EMPTY 3
#define QI_READ 0
#define QI_WRITE 1

/* Global variables. */

static struct {
    uint8_t operation; /* Read or write. */
    uint8_t data; /* Command if writing, response if reading. */
    uint8_t expected_res; /* The response received if command is successful. */
} queue_item;

static uint8_t cmd_queue[KB_QUEUE_SIZE];
static uint8_t *queue_head;
static uint8_t *queue_tail;

/* Static functions. */

static void reset_queue() {
    queue_tail = NULL;
    queue_head = cmd_queue;
    memset(cmd_queue, 0, KB_QUEUE_SIZE); /* Zero queue. */
}

static int queue(uint8_t item) {
    uint8_t *shift;
    int i;

    if (!queue_tail) /* No tail exists. */
        if (queue_head != cmd_queue) /* If head is not at beginning. */
            return EXIT_FAILURE;
        else /* Head is at correct place. */
            queue_tail = queue_head; /* Put tail at correct place. */

    /* Check if the tail is at the end. */
    if (queue_tail == cmd_queue + KB_QUEUE_SIZE) {
        if (queue_head == cmd_queue) /* Queue is full. */
            return KB_QUEUE_FULL;
        else { /* Queue is not full. */
            /* Shift all element down the queue. */
            shift = queue_head;

            for (i = 0; shift <= queue_tail; i++)
                cmd_queue[i] = *shift++;

            queue_head = cmd_queue; /* Reset head. */
            queue_tail -= --i; /* Move tail to new position. */
        }
    }

    *queue_tail++ = item;

    return EXIT_SUCCESS;
}

static int dequeue(uint8_t *item) {

    if (queue_head == cmd_queue + KB_QUEUE_SIZE) { /* Queue is empty. */
        reset_queue();
        return KB_QUEUE_EMPTY;
    }
    else if (queue_head > queue_tail) { /* Queue is empty. */
        reset_queue();
        return KB_QUEUE_EMPTY;
    }

    *item = *queue_head++; /* Pop head. */
    return EXIT_SUCCESS;
}

static uint8_t resend_cmd(uint8_t code) {
    int i, res;

    i = 0; /* Assumes one resend has already been sent. */

    /* Try to resend data three times if keyboard asks for a resend. */
    while ((res = PS2_read()) == KB_RESEND_CMD && i++ < KB_TIMEOUT)
        PS2_write(code);

    return res;
}

static int exec_cmds() {
    uint8_t cmd;
    int res;

    /* Execute all commands in the queue. */
    while (dequeue(&cmd) == EXIT_SUCCESS) {
        res = PS2_write(cmd);
    }

    res = PS2_read(); /* Check for ACK. */
    if (res == KB_RESEND_CMD) { /* If keyboard is requesting a resend. */
        res = resend_cmd(cmd); /* Try to resend until timeout reached. */

        if (res == KB_RESEND_CMD) /* Keyboard does not support command. */
            return KB_TIMEOUT_EXCEEDED;
    }

    return res;
}

/* External functions. */

extern int KB_reset(){
    int res;

    PS2_write(KB_RESET_TEST);
    res = PS2_read();
    if (res == KB_RESEND_CMD) {
        PS2_write(KB_RESET_TEST);
        res = PS2_read();
    }
    if (res == KB_CMD_ACK) {
        res = PS2_read();
    }
    else
        return res;

    if (res == KB_PASSED_SELF_TEST)
        return EXIT_SUCCESS;
    else
        return res;
}

extern int KB_enable(){
    int res;

    //queue(KB_ENABLE_SCANNING); /* Enable the keyboard. */
    //res = exec_cmds();
    PS2_write(KB_ENABLE_SCANNING);
    res = PS2_read();

    if (res == KB_CMD_ACK)
        return EXIT_SUCCESS;
    else
        return res;
}

extern int KB_disable(){
    int res;

    queue(KB_DISABLE_SCANNING); /* Disable the keyboard. */
    res = exec_cmds();

    if (res == KB_CMD_ACK)
        return EXIT_SUCCESS;
    else
        return res;
}

extern int KB_set_scan_code(uint8_t code){
    int res;

    /* Check for valid scan codes. */
    if (code > KB_SCAN_CODE_3 || code < KB_SCAN_CODE_1)
        return KB_INVALID_SCAN_CODE;

    /* Add commands to queue. */
    PS2_write(KB_GET_SET_SCAN_CODE);
    res = PS2_read();
    PS2_write(code);
    res = PS2_read();

    if (res == KB_CMD_ACK) /* Keyboard successfully received code. */
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

extern uint8_t KB_get_scan_code(){
    int res;

    PS2_write(KB_GET_SET_SCAN_CODE);
    res = PS2_read();
    PS2_write(KB_GET_SCAN_CODE);
    res = PS2_read();

    if (res == KB_CMD_ACK) /* Keyboard successfully received code. */
        return PS2_read(); /* Return scan code. */
    else /* Keyboard did not receive or does not support the code. */
        return res;
}

extern int KB_init(){
    int res;

    /* Reset keyboard. */
    res = KB_reset();
    if (res != EXIT_SUCCESS)
        return res;

    /* Set keyboard to a known scan code. */
    res = KB_set_scan_code(KB_DEFAULT_SCAN_CODE); /* Default scan code. */
    if (res != EXIT_SUCCESS)
        return res;

    /* 
     * Will need to check what scan code set is actually being used since
     * keyboard may not support the code set we set. 
     */

    /* Enable keyboard. */
    res = KB_enable();
    if (res != EXIT_SUCCESS)
        return res;

    return EXIT_SUCCESS;
}

extern keypress KB_get_keypress() {
    keypress key;
    uint8_t res;

    key.modifiers = 0;
    key.toggles = 0;
    res = PS2_read();

    /* Check for toggles and modifiers. */
    while (res <= KEY_SCROLL_LOCK && res >= KEY_L_SHIFT) {
        if (res <= KEY_R_ALT && res >= KEY_L_SHIFT) /* Modifiers. */
            key.modifiers = res;
        else if (res <= KEY_SCROLL_LOCK && res >= KEY_CAPS_LOCK) /* Toggles. */
            key.toggles = res;

        res = PS2_read();
    }

    key.codepoint = KB_code_set_1[res + 1]; /* Character key. */

    /* Check to see if key is released or pressed. */
    if (res - KB_KEY_RELEASE_OFFSET >= 0)
        key.pressed = 1;
    else
        key.pressed = 0;

    return key;
}

extern keypress KB_wait_for_scan_code() {
    keypress key;
    uint8_t res;
    //int i;

    res = PS2_read();
    key.codepoint = KB_code_set_1[res + 1];
    return key;
    /*
    while (1) {

        if (KB_code_set_1[res] >= KEY_L_SHIFT && KB_code_set_1[res] <= 
         KEY_R_ALT) {
            key.modifiers = KB_code_set_1[res];
            key.toggles = 0;
            res = PS2_read();
        }
        else if (KB_code_set_1[res] >= KEY_CAPS_LOCK && KB_code_set_1[res] <= 
         KEY_SCROLL_LOCK) {
            key.toggles = KB_code_set_1[res];
            key.modifiers = 0;
            res = PS2_read();
        }

        if (res < KB_KEY_RELEASE_OFFSET) {
            key.codepoint = KB_code_set_1[res];
            key.pressed = KEY_PRESSED;
        }
        else {
            key.codepoint = KB_code_set_1[res - KB_KEY_RELEASE_OFFSET];
            key.pressed = KEY_RELEASED;
        }

    }
    */
}

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
