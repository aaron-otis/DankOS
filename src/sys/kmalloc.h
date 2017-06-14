#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

void *kmalloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t _size);

#endif
