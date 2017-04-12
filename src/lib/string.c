#include "string.h"

#define STR_END '\0'

extern void *memset(void *dst, int c, size_t n) {
    char *d = dst;
    int i;

    for (i = 0; i < n; i++)
        d[i] = (char) c;

    return dst;
}

extern void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    int i;

    for (i = 0; i < n; i++)
        d[i] = s[i];

    return (void *) src;
}

extern size_t strlen(const char *s) {
    size_t len = 0;
    int i;

    for (i = 0; s[i] != STR_END; i++, len++)
        ; /* Iterate through |s| until null character is found. */

    return len;
}

extern char *strcpy(char *dest, const char *src) {
    int i;

    for (i = 0; src[i] != STR_END; i++)
        dest[i] = src[i];

    dest[i] = STR_END;

    return dest;
}

extern int strcmp(const char *s1, const char *s2) {
    int i = 0;

    while (s1[i] != STR_END && s2[i] != STR_END && s1[i] != s2[i])
        i++;

    return s1[i] - s2[i];
}

extern const char *strchr(const char *s, int c) {
    int i;

    for (i = 0; s[i] != STR_END; i++) {
        if (s[i] == (char) c)
            return s + i;
    }

    if (STR_END == (char) c)
        return s + i;
    else
        return NULL;
}

extern char *strdup(const char *s) {
    return NULL; /* Must implement malloc before this can be done. */
}
