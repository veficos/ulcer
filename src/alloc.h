

#ifndef _ULCER_ALLOC_H_
#define _ULCER_ALLOC_H_

#include "config.h"

#if (defined(_WIN32) || defined(WIN32)) && defined(DEBUG)
#include <malloc.h>
#   define heap_calloc(n)   calloc(n, sizeof(char))   
#   define heap_alloc       malloc   
#   define heap_realloc     realloc  
#   define heap_free        free
#else 
    void *heap_alloc(unsigned long size);
    void *heap_calloc(unsigned long size);
    void *heap_realloc(void *ptr, unsigned long size);
    void heap_free(void *ptr);
#endif

#endif