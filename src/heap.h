

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
object_t heap_alloc_function(environment_t env, expression_function_t function_expr);
object_t heap_alloc_native_function(environment_t env, native_function_pt native_function);
void     heap_hold_value(environment_t env, value_t v);
void     heap_drop_value(environment_t env, value_t v);

#endif