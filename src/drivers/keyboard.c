#include "keyboard.h"

#define KB_TIMEOUT 3
#define KB_DEFAULT_SCAN_CODE KB_SCAN_CODE_2
#define KB_QUEUE_FULL 2
#define KB_QUEUE_EMPTY 3
#define QI_READ 0
#define QI_WRITE 1

#define LAST_KEY 0x46

/* Global variables. */

static struct {
    uint8_t operation; /* Read or write. */
    uint8_t data; /* Command if writing, response if reading. */
    uint8_t expected_res; /* The response received if command is successful. */
} queue_item;

static uint8_t cmd_queue[KB_QUEUE_SIZE];
static uint8_t *queue_head;
static uint8_t *queue_tail;
static int modifiers_pressed;
static int toggles_set;

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

    modifiers_pressed = 0;
    toggles_set = 0;

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
    char c;

    key.codepoint = 0; /* Zero this in case characters are not printable. */
    res = PS2_read();


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
