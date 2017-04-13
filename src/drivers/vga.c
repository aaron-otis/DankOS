#include "vga.h"
#include "../lib/string.h"
#include "../lib/stdio.h"

#define NEWLINE '\n'
#define VGA_PTR ((void *) VGA_ADDR)
#define VGA_BUF_LEN (SCREEN_WIDTH * SCREEN_HEIGHT)
#define VGA_BG_SHIFT 4
#define VGA_BLINK_SHIFT 7

static int buffer_pos;
static int cursor_pos;
static unsigned short *vga_buff = VGA_PTR;
static unsigned char attributes;

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

    attributes = (blink << VGA_BLINK_SHIFT) | (bg << VGA_BG_SHIFT) | fg;

    return attributes >= 0;
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
    
    if (c == NEWLINE) { /* Handle a newline character. */
        /* Clear cursor attributes. */
        vga_buff[buffer_pos] = attributes << CHAR_BIT;
        vga_buff[cursor_pos] = attributes << CHAR_BIT;

        /* Advance to new position in the buffer. */
        buffer_pos += SCREEN_WIDTH - (buffer_pos % SCREEN_WIDTH);
        cursor_pos = buffer_pos;
    }
    else {
        /* Add character to the buffer. */
        vga_buff[buffer_pos++] = (attributes << CHAR_BIT) | c;
        cursor_pos++;
    }

    /* Scroll screen if buffer position is past buffer. */
    if (buffer_pos >= VGA_BUF_LEN)
        scroll_screen();

    /* Set cursor attributes. */
    vga_buff[cursor_pos] |= (~attributes) << CHAR_BIT;
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
    vga_buff[cursor_pos] |= attributes << CHAR_BIT;

    if (pos > 0 && pos < VGA_BUF_LEN)
        cursor_pos = pos; /* Set new cursor position. */
    else
        return EXIT_FAILURE;

    /* set cursor attributes. */
    vga_buff[cursor_pos] |= (~attributes) << CHAR_BIT;

    return EXIT_SUCCESS;
}

extern int VGA_get_cur_pos() {

    return cursor_pos;
}

extern int VGA_get_buf_pos() {

    return buffer_pos;
}
