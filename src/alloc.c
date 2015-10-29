

#include "error.h"

#include <malloc.h>

void* heap_alloc(unsigned long size)
{
    void *ptr = malloc(size);
    if (!ptr) {
        error(NULL, 0, 0, "out of memory");
    }
    return ptr;
}

void *heap_realloc(void *ptr, unsigned long size)
{
    void *reptr = realloc(ptr, size);
    if (!reptr) {
        error(NULL, 0, 0, "out of memory");
    }
    return reptr;
}

void *heap_calloc(unsigned long size)
{
    void *ptr = calloc(size, 1);
    if (!ptr) {
        error(NULL, 0, 0, "out of memory");
    }
    return ptr;
}

void heap_free(void *ptr)
{
    free(ptr);
}