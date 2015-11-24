

#include "error.h"

#include <malloc.h>

void* mem_alloc(unsigned long size)
{
    void *ptr = malloc(size);
    if (!ptr) {
        runtime_error("out of memory");
    }
    return ptr;
}

void *mem_realloc(void *ptr, unsigned long size)
{
    void *reptr = realloc(ptr, size);
    if (!reptr) {
        runtime_error("out of memory");
    }
    return reptr;
}

void *mem_calloc(unsigned long size)
{
    void *ptr = calloc(size, 1);
    if (!ptr) {
        runtime_error("out of memory");
    }
    return ptr;
}

void mem_free(void *ptr)
{
    free(ptr);
}