#ifndef __VGA_H
#define __VGA_H

#include <limits.h>
#include "../lib/stdlib.h"
#include "../lib/stdint.h"

/* Buffer constants. */
#define VGA_ADDR 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/* Color constants. */
#define VGA_BLACK 0x0
#define VGA_BLUE 0x1
#define VGA_GREEN 0x2
#define VGA_CYAN 0x3
#define VGA_RED 0x4
#define VGA_MAGENTA 0x5
#define VGA_BROWN 0x6
#define VGA_GRAY 0x7
#define VGA_DARK_GRAY 0x8
#define VGA_BRIGHT_BLUE 0x9
#define VGA_BRIGHT_GREEN 0xA
#define VGA_BRIGHT_CYAN 0xB
#define VGA_BRIGHT_RED 0xC
#define VGA_BRIGHT_MAGENTA 0xD
#define VGA_YELLOW 0xE
#define VGA_WHITE 0xF

/* Attributes. */
#define BLINKING (1 << 7)

extern int VGA_init(void);
extern int VGA_set_attr(char fg, char bg, char blink);
extern char VGA_get_attr();
extern void VGA_clear(void);
extern void VGA_display_char(char c);
extern void VGA_display_str(const char *s);
extern int VGA_set_cursor_pos(int pos);
extern int VGA_get_cur_pos();
extern int VGA_get_buf_pos();
extern void VGA_disable_cursor();
extern void VGA_enable_cursor(char fg, char bg, char blink);
extern void VGA_backspace();
int VGA_row_count(void);
int VGA_col_count(void);
void VGA_display_attr_char(int x, int y, char c, int fg, int bg);

#endif
