
#include "environment.h"
#include "cstring.h"
#include "alloc.h"
#include "array.h"
#include "stack.h"
#include "heap.h"

struct heap_s {
    long   allocated;
    long   threshold;
    list_t objects;
};

#ifndef HEAP_THRESHOLD_SIZE
#define HEAP_THRESHOLD_SIZE (1024*256)
#endif

#define __heap_value_is_object__(value)                                       \
    ((value).type == VALUE_TYPE_STRING)

static object_t __heap_alloc_object__(environment_t env, object_type_t type);
static void __heap_need_gc__(environment_t env);
static void __heap_mark_objects__(environment_t env);
static void __heap_sweep_objects__(environment_t env);
static void __heap_dispose_object__(object_t obj);
static void __heap_unmark_object__(object_t obj);
static void __heap_mark_object__(object_t obj);

heap_t heap_new(void)
{
    heap_t heap = (heap_t) mem_alloc(sizeof(struct heap_s));
    if (!heap) {
        return NULL;
    }

    heap->allocated      = 0;
    heap->threshold = HEAP_THRESHOLD_SIZE;

    list_init(heap->objects);

    return heap;
}

void heap_free(heap_t heap)
{
    list_iter_t iter, next_iter;
    object_t object;

    list_safe_for_each(heap->objects, iter, next_iter) {
        object = list_element(iter, object_t, link);
        list_erase(*iter);
        __heap_dispose_object__(object);
    }

    mem_free(heap);
}

void heap_gc(environment_t env)
{
    __heap_mark_objects__(env);
    __heap_sweep_objects__(env);
}

void heap_takeover_value(environment_t env, value_t value)
{
    if (__heap_value_is_object__(*value)) {
        value->u.object_value->marked = false;
        list_push_back(env->heap->objects, value->u.object_value->link);
    }
}

object_t heap_alloc_string(environment_t env, cstring_t cstr)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_STRING);

    object->u.string = cstring_dup(cstr);

    return object;
}

static object_t __heap_alloc_object__(environment_t env, object_type_t type)
{
    object_t object;

    __heap_need_gc__(env);

    object = mem_alloc(sizeof(struct object_s));
    if (!object) {
        return NULL;
    }

    object->type   = type;
    object->marked = false;

    list_push_back(env->heap->objects, object->link);

    return object;
}

#include <stdio.h>
static void __heap_need_gc__(environment_t env)
{
    /*
    if (env->heap->allocated > env->heap->threshold) {
        heap_gc(env);
    }
    */

    heap_gc(env);
}

static void __heap_mark_objects__(environment_t env)
{
    
    {
        /* clear mark */
        list_iter_t iter;
        object_t object;

        list_for_each(env->heap->objects, iter) {
            object = list_element(iter, object_t, link);
            __heap_unmark_object__(object);
        }
    }

    {
        /* mark global variable */
        global_variable_t variable;
        hash_table_iter_t iter;

        iter = hash_table_iter_new(env->global_context);
        hash_table_for_each(env->global_context, iter) {
            variable = hash_table_iter_element(iter, global_variable_t, link);
            if (__heap_value_is_object__(*variable->value)) {
                __heap_mark_object__(variable->value->u.object_value);
            }           
        }
        hash_table_iter_free(iter);
    }

    {
        /* mark stack */
        int index;
        array_t stack;
        value_t stack_base;
        
        stack = env->stack;
        array_for_each(stack, stack_base, index) {
            if (__heap_value_is_object__(stack_base[index])) {
                __heap_mark_object__(stack_base[index].u.object_value);
            }
        }
    }
}

static void __heap_sweep_objects__(environment_t env)
{
    list_iter_t iter, next_iter;
    object_t object;

    list_safe_for_each(env->heap->objects, iter, next_iter) {
        object = list_element(iter, object_t, link);
        if (!object->marked) {
            list_erase(*iter);
            __heap_dispose_object__(object);
        }
    }
}

static void __heap_dispose_object__(object_t obj)
{
    if (obj->type == OBJECT_TYPE_STRING) {
        cstring_free(obj->u.string);
    }

    mem_free(obj);
}

static void __heap_unmark_object__(object_t obj)
{
    obj->marked = false;
}

static void __heap_mark_object__(object_t obj)
{
    if (obj->marked) {
        return ;
    }

    obj->marked = true;
}