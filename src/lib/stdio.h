#ifndef __STDIO_H
#define __STDIO_H

/* Must support %% %d %u %x %c %p %h[dux] %l[dux] %q[dux] %s at minimum. */

/* Return values. */
#define STDIO_INVALID_LENGTH -2

/* Conversion Specifiers. */
#define FMT_DELIM '%'
#define INT_DELIM 'd'
#define UINT_DELIM 'u'
#define HEX_DELIM 'x'
#define CHAR_DELIM 'c'
#define PTR_DELIM 'p'
#define STR_DELIM 's'

/* Length modifiers. */
#define HALF_MODIFIER 'h'
#define LONG_MODIFIER 'l'
#define QUAD_MODIFIER 'q'

/* extern void putc(char c); */
__attribute__ ((format (printf, 1, 2))) extern int printk(const char *fmt, 
 ...) ;

#endif
