#include <stdarg.h>
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "../drivers/vga.h"

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

static unsigned int modifier;

void print_char(char c) {
    VGA_display_char(c);
}

int build_string(char **str, unsigned long long val, int base) {
    int len, i;

    /* Determine the string length. */
    if (modifier == LONG_ARG)
        len = LONG_STR_LEN;
    else if (modifier == SHORT_ARG)
        len = SHORT_STR_LEN;
    else if (modifier == QUAD_ARG)
        len = QUAD_STR_LEN;
    else
        len = INT_STR_LEN;

    /* Get individual digits. */
    str[len] = 0;
    for (; val > 0 && len; val /= base) {
        (*str)[--len] = (val % base) + ASCII_ZERO;

        if (len < 0)
            return STDIO_INVALID_LENGTH;
    }

    /* Adjust string for printing. */
    if (len > 0) {
        for (i = 0; i < len; i++)
            (*str)[i] = (*str)[i + len];

        (*str)[len] = 0;
    }

    return EXIT_SUCCESS;
}

void print_int(int i) {
    int len;
    char buf[LONG_STR_LEN + 1], *str;

    str = buf;
    str++;
    build_string(&str, i, DECIMAL_BASE);

    if (i < 0)
        *--str = '-';

    VGA_display_str(str);
}

void print_uint(unsigned int u) {
    int len, i;
    char str[LONG_STR_LEN];
    char *buf;

    /* Determine the string length. */
    /*
    if (modifier == LONG_ARG)
        len = LONG_STR_LEN;
    else if (modifier == SHORT_ARG)
        len = SHORT_STR_LEN;
    else if (modifier == QUAD_ARG)
        len = QUAD_STR_LEN;
    else
        len = INT_STR_LEN;

    str[len] = 0;
    for (; u > 0 && len; u /= DECIMAL_BASE) {
        str[--len] = (u % DECIMAL_BASE) + ASCII_ZERO;
    }

    if (len > 0) {
        for (i = 0; i < len; i++)
            str[i] = str[i + len];

        str[len] = 0;
    }
    */
    buf = str;
    build_string(&buf, u, DECIMAL_BASE);

    VGA_display_str(str);
}

void print_hex(unsigned int u) {
}

void print_ptr(void *p) {
}

void print_str(char *str) {
    VGA_display_str(str);
}


__attribute__ ((format (printf, 1, 2))) extern int printk(const char *fmt, 
 ...) {
    int i;
    va_list ap;

    va_start(ap, fmt);

    for (i = 0; i < strlen(fmt); i++) {
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
            }

            /* Check for conversion specifiers. */
            switch (fmt[i]) {
                case FMT_DELIM:
                    print_char(fmt[i]);
                    break;
                
                case INT_DELIM:
                    print_int(va_arg(ap, int));
                    break;

                case UINT_DELIM:
                    print_uint(va_arg(ap, unsigned int));
                    break;

                case HEX_DELIM:
                    print_hex(va_arg(ap, unsigned int));
                    break;

                case CHAR_DELIM:
                    print_char(va_arg(ap, int));
                    break;

                case PTR_DELIM:
                    print_ptr(va_arg(ap, void *));
                    break;

                case STR_DELIM:
                    print_str(va_arg(ap, char *));
                    break;

                default:
                    return -1;
            }
        }
        else
            print_char(fmt[i]);
    }

    va_end(ap);

    return EXIT_SUCCESS;
}
