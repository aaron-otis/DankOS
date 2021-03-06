#include <stdarg.h>
#include <limits.h>
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/interrupts.h"

#define LONG_ARG 1
#define SHORT_ARG 2
#define QUAD_ARG 4
#define NORMAL_ARG 0

#define SHORT_STR_LEN 5
#define INT_STR_LEN 10
#define LONG_STR_LEN 20
#define QUAD_STR_LEN 20

#define DECIMAL_BASE 10
#define HEX_BASE 16
#define ASCII_ZERO '0'
#define HEX_LOWER_START 'a'

#define POS_SIGN 1
#define NEG_SIGN -1
#define SHORT_MASK 0xFFFF

/* Flag characters. */
#define NO_FLAGS 0
#define PAD_FLAG '0'
#define LEFT_ADJ_FLAG '-'
#define PAD_POS_FLAG ' '
#define SIGN_FLAG '+'

#define BACKSPACE '\b'

static unsigned int modifier;

void print_char(char c) {

    if (c == BACKSPACE)
        VGA_backspace();
    else {
        VGA_display_char(c);
        SER_putc(c);
    }
}

static int build_string(char **str, unsigned long long val, int base, int sign) {
    int len, i, size, ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    /* Determine the string length. */
    if (modifier == LONG_ARG) {
        len = LONG_STR_LEN;
    }
    else if (modifier == SHORT_ARG)
        len = SHORT_STR_LEN;
    else if (modifier == QUAD_ARG)
        len = QUAD_STR_LEN;
    else
        len = INT_STR_LEN;

    size = len;

    if (sign == NEG_SIGN)
        val = ~val + 1;

    /* Get individual digits. */
    (*str)[len] = 0;
    if (val == 0)
        (*str)[--len] = '0';
    else {
        for (; val > 0 && len; val /= base) {
            (*str)[--len] = (val % base);

            if (len < 0) {
                if (ints_enabled)
                    STI;

                return STDIO_INVALID_LENGTH;
            }

            /* Convert value to the correct character. */
            if ((*str)[len] >= DECIMAL_BASE)
                (*str)[len] += HEX_LOWER_START - DECIMAL_BASE;
            else
                (*str)[len] += ASCII_ZERO;
        }
    }

    /* Adjust string for printing. */
    if (len > 0) {
        for (i = 0; i < size; i++)
            (*str)[i] = (*str)[i + len];

        (*str)[size - len] = 0;
    }

    if (ints_enabled)
        STI;

    return strlen(*str);
}

static int print_int(int i) {
    int sign;
    char buf[LONG_STR_LEN + 1], *str;

    str = buf;
    if (i < 0) { /* Check if integer is negative. */
        *str = '-';
    }
    else
        *str = '+';
    str++;
    sign = i < 0 ? NEG_SIGN : POS_SIGN;

    if (0 > build_string(&str, i, DECIMAL_BASE, sign))
        VGA_display_str("\nprint_int error\n");

    if (i < 0)
        str--;

    VGA_display_str(str);
    SER_write(str, strlen(str));

    return strlen(str);
}

static int print_uint(unsigned int u) {
    char str[LONG_STR_LEN];
    char *buf;
    int len;

    buf = str;
    len = build_string(&buf, u, DECIMAL_BASE, POS_SIGN);
    if (0 > len)
        VGA_display_str("\nprint_uint error\n");

    VGA_display_str(str);
    SER_write(str, strlen(str));
    return len;
}

static int print_hex(unsigned int u) {
    char str[LONG_STR_LEN], *buf;

    buf = str;
    if (0 > build_string(&buf, u, HEX_BASE, POS_SIGN))
        VGA_display_str("\nprint_uint error\n");

    VGA_display_str(str);
    SER_write(str, strlen(str));
    return strlen(str);
}

static int print_ptr(void *p) {
    char str[LONG_STR_LEN + 2], *buf;
    int ints_enabled = 0;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    modifier = LONG_ARG;
    str[0] = '0';
    str[1] = 'x';
    buf = str + 2;

    if (0 > build_string(&buf, (unsigned long) p, HEX_BASE, POS_SIGN))
        VGA_display_str("\nprint_ptr error\n");

    VGA_display_str(str);
    SER_write(str, strlen(str));

    if (ints_enabled)
        STI;

    return strlen(str);
}

static int print_str(char *str) {
    VGA_display_str(str);
    SER_write(str, strlen(str));

    return strlen(str);
}


extern int printk(const char *fmt, ...) {
    int i, len = 0, ints_enabled = 0, fmt_len = strlen(fmt);
    va_list ap;

    if (are_interrupts_enabled()) {
        ints_enabled = 1;
        CLI;
    }

    va_start(ap, fmt);

    for (i = 0; i < fmt_len; i++) {
        if (fmt[i] == FMT_DELIM) {
            i++;

            /* Check for length modifiers. */
            switch (fmt[i]) {
                case HALF_MODIFIER:
                    modifier = SHORT_ARG;
                    i++;
                    break;
                    
                case LONG_MODIFIER:
                    modifier = LONG_ARG;
                    i++;
                    break;

                case QUAD_MODIFIER:
                    modifier = QUAD_ARG;
                    i++;
                    break;
                default:
                    modifier = NORMAL_ARG;
                    break;
            }

            /* Check for conversion specifiers. */
            switch (fmt[i]) {
                case FMT_DELIM:
                    print_char(fmt[i]);
                    break;
                
                case INT_DELIM:
                    if (modifier == SHORT_ARG)
                        len += print_int(va_arg(ap, int));
                    else if (modifier == LONG_ARG)
                        len += print_int(va_arg(ap, long int));
                    else if (modifier == QUAD_ARG)
                        len += print_int(va_arg(ap, long long int));
                    else
                        len += print_int(va_arg(ap, int));
                    break;

                case UINT_DELIM:
                    if (modifier == SHORT_ARG)
                        len += print_uint(va_arg(ap, unsigned int));
                    else if (modifier == LONG_ARG)
                        len += print_uint(va_arg(ap, unsigned long int));
                    else if (modifier == QUAD_ARG)
                        len += print_uint(va_arg(ap, unsigned long long int));
                    else
                        len += print_uint(va_arg(ap, unsigned int));

                    break;

                case HEX_DELIM:
                    if (modifier == SHORT_ARG)
                        len += print_hex(va_arg(ap, unsigned int));
                    else if (modifier == LONG_ARG)
                        len += print_hex(va_arg(ap, unsigned long int));
                    else if (modifier == QUAD_ARG)
                        len += print_hex(va_arg(ap, unsigned long long int));
                    else
                        len += print_hex(va_arg(ap, unsigned int));
                    break;

                case CHAR_DELIM:
                    print_char(va_arg(ap, int));
                    len++;
                    break;

                case PTR_DELIM:
                    len += print_ptr(va_arg(ap, void *));
                    break;

                case STR_DELIM:
                    len += print_str(va_arg(ap, char *));
                    break;

                default:
                    if (ints_enabled)
                        STI;

                    return -1;
            }
        }
        else {
            print_char(fmt[i]);
            len++;
        }
    }

    va_end(ap);

    if (ints_enabled)
        STI;

    return len;
}
