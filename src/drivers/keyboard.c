#include "keyboard.h"

static uint8_t scan_code;
static uint8_t cmd_queue[KB_QUEUE_SIZE];
static uint8_t *queue_head;
static uint8_t *queue_tail;

static void reset_queue() {
    queue_tail = NULL;
    queue_head = cmd_queue;
}

static int queue(uint8_t item) {
    uint8_t *shift;
    int i;

    if (!queue_tail)
        if (queue_head != cmd_queue)
            return EXIT_FAILURE;
        else
            queue_tail = queue_head;

    /* Check if the tail is at the end. */
    if (queue_tail == cmd_queue + KB_QUEUE_SIZE) {
        if (queue_head == cmd_queue) /* Queue is full. */
            return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }
    else if (queue_head > queue_tail) { /* Queue is empty. */
        reset_queue();
        return EXIT_FAILURE;
    }

    *item = *queue_head++; /* Pop head. */
    return EXIT_SUCCESS;
}

extern int KB_reset(){
}

extern int KB_enable(){
}

extern int KB_disable(){
}

extern int KB_set_scan_code(uint8_t code){

    scan_code = code;
}

extern int KB_get_scan_code(){

}

extern int KB_init(){

    reset_queue();
    memset(cmd_queue, 0, KB_QUEUE_SIZE); /* Zero queue. */

    /* Reset keyboard. */
    if (KB_reset() == EXIT_FAILURE)
        return EXIT_FAILURE;

    /* Set keyboard to a known scan code. */
    KB_set_scan_code(KB_SCAN_CODE_2); /* Default scan code. */

    /* Enable keyboard. */
    if (KB_enable() == EXIT_FAILURE)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

extern int KB_wait_for_scan_code() {
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
