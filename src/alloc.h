

#ifndef _ULCER_ALLOC_H_
#define _ULCER_ALLOC_H_

#include "config.h"

#if (defined(_WIN32) || defined(WIN32)) && defined(DEBUG)
#include <malloc.h>
#   define mem_calloc(n)   calloc(n, sizeof(char))
#   define mem_alloc       malloc
#   define mem_realloc     realloc
#   define mem_free        free
#else 
    void *mem_alloc(unsigned long size);
    void *mem_calloc(unsigned long size);
    void *mem_realloc(void *ptr, unsigned long size);
    void  mem_free(void *ptr);
#endif

#endif