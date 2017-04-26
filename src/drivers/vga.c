#include "vga.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "interrupts.h"

#define NEWLINE '\n'
#define VGA_PTR ((void *) VGA_ADDR)
#define VGA_BUF_LEN (SCREEN_WIDTH * SCREEN_HEIGHT)
#define VGA_BG_SHIFT 4
#define VGA_BLINK_SHIFT 7
#define LOWER_MASK 0xFF

typedef struct {
    uint8_t fg:4;
    uint8_t bg:3;
    uint8_t blink:1;
} attributes;

static int buffer_pos;
static int cursor_pos;
static uint16_t *vga_buff = VGA_PTR;
static attributes text_attr;
static attributes cursor_attr;


static int scroll_screen() {
    int i;
    void *last_line = vga_buff + VGA_BUF_LEN - SCREEN_WIDTH;

    /* Copy everything SCREEN_WIDTH bytes back. */
    for (i = SCREEN_WIDTH; i < VGA_BUF_LEN; i++)
        vga_buff[i - SCREEN_WIDTH] = vga_buff[i];

    /* 
     * Clear the last line of the VGA buffer. Characters in the buffer are
     * 16-bits, so double the amount of SCREEN_WIDTH is needed.
     */
    memset(last_line, 0, SCREEN_WIDTH * 2);

    /* Set buffer and cursor positions to new positions. */
    buffer_pos -= SCREEN_WIDTH;
    cursor_pos -= SCREEN_WIDTH;

    return EXIT_SUCCESS;
}

extern int VGA_set_attr(char fg, char bg, char blink) {

    //attributes = (blink << VGA_BLINK_SHIFT) | (bg << VGA_BG_SHIFT) | fg;
    text_attr.blink = blink;
    text_attr.fg = fg;
    text_attr.bg = bg;

    cursor_attr.blink = 0;
    cursor_attr.fg = ~fg;
    cursor_attr.bg = ~bg;

    return EXIT_SUCCESS;
}

extern char VGA_get_attr() {

    return *(char *) &text_attr;
}

extern void VGA_clear(void) {

    /* Set all bytes in video memory to zero. */
    memset(VGA_PTR, 0, VGA_BUF_LEN * 2);
}

extern int VGA_init(void) {
    VGA_clear(); /* Clear screen. */
    buffer_pos = cursor_pos = 0; /* Initial position. */

    return EXIT_SUCCESS;
}

extern void VGA_display_char(char c) {
    int ints_enabled = 0;

    /* If interrupts are enabled, disable them. */
    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }
    
    if (c == NEWLINE) { /* Handle a newline character. */
        /* Clear cursor attributes. */
        vga_buff[buffer_pos] = (*(char *) &text_attr) << CHAR_BIT;
        vga_buff[cursor_pos] = (*(char *) &text_attr) << CHAR_BIT;

        /* Advance to new position in the buffer. */
        buffer_pos += SCREEN_WIDTH - (buffer_pos % SCREEN_WIDTH);
        cursor_pos = buffer_pos;
    }
    else {
        /* Add character to the buffer. */
        vga_buff[buffer_pos++] = ((*(char *) &text_attr) << CHAR_BIT) | c;
        cursor_pos++;
    }

    /* Scroll screen if buffer position is past buffer. */
    if (buffer_pos >= VGA_BUF_LEN)
        scroll_screen();

    /* Set cursor attributes. */
    vga_buff[cursor_pos] |= (*(char *) &cursor_attr) << CHAR_BIT;

    if (ints_enabled) /* If interrupts were initially enabled, enable them. */
        STI;
}

/*
 * TODO: Copy dynamic 16-bit character array directly into VGA buffer
 * after malloc has been implemented.
 */
extern void VGA_display_str(const char *s) {
    int i;

    for (i = 0; i < strlen(s); i++)
        VGA_display_char(s[i]);
}

extern int VGA_set_cursor_pos(int pos) {

    /* Clear cursor attributes. */
    vga_buff[cursor_pos] &= LOWER_MASK;
    vga_buff[cursor_pos] |= (*(char *) &text_attr) << CHAR_BIT;

    if (pos > 0 && pos < VGA_BUF_LEN)
        cursor_pos = pos; /* Set new cursor position. */
    else
        return EXIT_FAILURE;

    /* set cursor attributes. */
    vga_buff[cursor_pos] |= (*(char *) &cursor_attr) << CHAR_BIT;

    return EXIT_SUCCESS;
}

extern int VGA_get_cur_pos() {

    return cursor_pos;
}

extern int VGA_get_buf_pos() {

    return buffer_pos;
}

extern void VGA_disable_cursor() {
    *(char *) &cursor_attr = 0;
    vga_buff[cursor_pos] &= LOWER_MASK;
    vga_buff[cursor_pos] |= (*(char *) &text_attr) << CHAR_BIT;
}

extern void VGA_enable_cursor(char fg, char bg, char blink) {
    cursor_attr.blink = blink;
    cursor_attr.fg = fg;
    cursor_attr.bg = bg;
    vga_buff[cursor_pos] &= LOWER_MASK;
    vga_buff[cursor_pos] |= (*(char *) &cursor_attr) << CHAR_BIT;
}

extern void VGA_backspace() {

    if (buffer_pos % SCREEN_WIDTH) {
        vga_buff[--buffer_pos] = ((*(char *) &text_attr) << CHAR_BIT) | 0;
        VGA_set_cursor_pos(cursor_pos - 1);
    }
}
