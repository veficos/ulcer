

#ifndef _ULCER_HEAP_H_
#define _ULCER_HEAP_H_

#include "config.h"

heap_t heap_new(void);
void heap_free(heap_t heap);
void heap_gc(environment_t env);
object_t heap_alloc_string(environment_t env, cstring_t cstr);
object_t heap_alloc_string_n(environment_t env, unsigned long n);
object_t heap_alloc_array(environment_t env);
object_t heap_alloc_array_n(environment_t env, unsigned long n);
object_t heap_alloc_local_context(environment_t env);

#endif