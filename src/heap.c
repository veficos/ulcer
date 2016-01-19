

#include "environment.h"
#include "alloc.h"
#include "list.h"
#include "heap.h"

#include <assert.h>

struct heap_s {
    long   allocated;
    long   threshold;
    list_t objects;
};

#ifndef HEAP_THRESHOLD_SIZE
#define HEAP_THRESHOLD_SIZE (1024*256)
#endif

#define __heap_value_is_object__(value)                                       \
    (((value)->type == VALUE_TYPE_STRING) || ((value)->type == VALUE_TYPE_ARRAY) || \
     ((value)->type == VALUE_TYPE_FUNCTION) ||  ((value)->type == VALUE_TYPE_NATIVE_FUNCTION) || \
     ((value)->type == VALUE_TYPE_TABLE)) 

static void     __heap_unmark_object__(object_t obj);
static void     __heap_mark_object__(object_t obj);
static void     __heap_dispose_object__(object_t obj);
static void     __heap_mark_objects__(environment_t env);
static void     __heap_sweep_objects__(environment_t env);
static object_t __heap_alloc_object__(environment_t env, object_type_t type);
static void     __heap_auto_gc__(environment_t env);

heap_t heap_new(void)
{
    heap_t heap = (heap_t) mem_alloc(sizeof(struct heap_s));
    if (!heap) {
        return NULL;
    }

    heap->allocated = 0;
    heap->threshold = HEAP_THRESHOLD_SIZE;

    list_init(heap->objects);

    return heap;
}

void heap_free(heap_t heap)
{
    list_iter_t iter, next_iter;
    object_t object;

    list_safe_for_each(heap->objects, iter, next_iter) {
        object = list_element(iter, object_t, link_heap);
        list_erase(heap->objects, *iter);
        __heap_dispose_object__(object);
    }

    mem_free(heap);
}

void heap_gc(environment_t env)
{
    __heap_mark_objects__(env);
    __heap_sweep_objects__(env);
}

object_t heap_alloc_str(environment_t env, const char* str)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_STRING);

    object->u.string = cstring_new(str);

    return object;
}

object_t heap_alloc_string(environment_t env, cstring_t cstr)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_STRING);

    object->u.string = cstring_dup(cstr);

    return object;
}

object_t heap_alloc_string_n(environment_t env, unsigned long n) 
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_STRING);

    object->u.string = cstring_newempty(n);

    return object;
}

object_t heap_alloc_array(environment_t env)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_ARRAY);

    object->u.array = array_new(sizeof(value_t));

    return object;
}

object_t heap_alloc_array_n(environment_t env, unsigned long n)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_ARRAY);

    object->u.array = array_newlen(sizeof(value_t), n);

    return object;
}

object_t heap_alloc_table(environment_t env)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_TABLE);

    object->u.table = table_new();

    return object;
}

object_t heap_alloc_function(environment_t env, expression_function_t function_expr)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_FUNCTION);

    object->u.function = mem_alloc(sizeof(struct function_s));

    object->u.function->f.function_expr = function_expr;
    
    list_init(object->u.function->scopes);

    return object;
}

object_t heap_alloc_native_function(environment_t env, native_function_pt native_function)
{
    object_t object = __heap_alloc_object__(env, OBJECT_TYPE_NATIVE_FUNCTION);

    object->u.function = mem_alloc(sizeof(struct function_s));

    object->u.function->f.native_function = native_function;

    list_init(object->u.function->scopes);

    return object;
}

static object_t __heap_alloc_object__(environment_t env, object_type_t type)
{
    object_t object;

    __heap_auto_gc__(env);

    object = mem_alloc(sizeof(struct object_s));
    if (!object) {
        return NULL;
    }

    object->type   = type;
    object->marked = false;

    list_push_back(env->heap->objects, object->link_heap);

    return object;
}

static void __heap_auto_gc__(environment_t env)
{
    heap_gc(env);
}

static void __heap_mark_objects__(environment_t env)
{
    
    {
        /* clear mark */
        list_iter_t iter;
        object_t object;

        list_for_each(env->heap->objects, iter) {
            object = list_element(iter, object_t, link_heap);
            __heap_unmark_object__(object);
        }
    }

    {
        /* mark global variable */
        table_pair_t variable;
        hash_table_iter_t iter;

        iter = hash_table_iter_new(env->global_table->table);
        hash_table_for_each(env->global_table->table, iter) {
            variable = hash_table_iter_element(iter, table_pair_t, link);

            if (__heap_value_is_object__(variable->key)) {
                __heap_mark_object__(variable->key->u.object_value);
            }

            if (__heap_value_is_object__(variable->value)) {
                __heap_mark_object__(variable->value->u.object_value);
            }
        }

        hash_table_iter_free(iter);
    }

    {
        /* mark stack */
        list_iter_t iter;
        value_t value;

        list_for_each(env->stack, iter) {
            value = list_element(iter, value_t, link);
            if (__heap_value_is_object__(value)) {
                __heap_mark_object__(value->u.object_value);
            }
        }
    }

    {
        /* mark local context */
        list_iter_t iter;
        local_context_t context;
        table_pair_t variable;
        hash_table_iter_t hiter;

        list_for_each(env->local_context_stack, iter) {
            context = list_element(iter, local_context_t, link);

            __heap_mark_object__(context->object);

            hiter = hash_table_iter_new(context->object->u.table->table);
            hash_table_for_each(context->object->u.table->table, hiter) {
                variable = hash_table_iter_element(hiter, table_pair_t, link);

                if (__heap_value_is_object__(variable->key)) {
                    __heap_mark_object__(variable->key->u.object_value);
                }

                if (__heap_value_is_object__(variable->value)) {
                    __heap_mark_object__(variable->value->u.object_value);
                }
            }

            hash_table_iter_free(hiter);
        }
    }
}

static void __heap_sweep_objects__(environment_t env)
{
    list_iter_t iter, next_iter;
    object_t object;

    list_safe_for_each(env->heap->objects, iter, next_iter) {
        object = list_element(iter, object_t, link_heap);
        if (!object->marked) {
            list_erase(env->heap->objects, *iter);
            __heap_dispose_object__(object);
        }
    }
}

static void __heap_dispose_object__(object_t obj)
{
    value_t *base;
    int index;

    switch (obj->type) {
    case OBJECT_TYPE_STRING:
        cstring_free(obj->u.string);
        break;

    case OBJECT_TYPE_ARRAY:
        array_for_each(obj->u.array, base, index) {
            value_free(base[index]);
        }
        array_free(obj->u.array);
        break;

    case OBJECT_TYPE_TABLE:
        table_free(obj->u.table);
        break;

    case OBJECT_TYPE_FUNCTION:

    case OBJECT_TYPE_NATIVE_FUNCTION:
        mem_free(obj->u.function);
        break;

    default:
        break;
    }

    mem_free(obj);
}

static void __heap_unmark_object__(object_t obj)
{
    obj->marked = false;
}

static void __heap_mark_object__(object_t obj)
{
    value_t *base;
    int index;
    list_iter_t iter;
    table_pair_t variable;
    hash_table_iter_t hiter;

    if (obj->marked) {
        return ;
    }

    obj->marked = true;

    switch (obj->type) {
    case OBJECT_TYPE_NATIVE_FUNCTION:
    case OBJECT_TYPE_FUNCTION:
        list_for_each(obj->u.function->scopes, iter) {
            object_t object = list_element(iter, object_t, link_scope);

            __heap_mark_object__(object);
            
            hiter = hash_table_iter_new(object->u.table->table);
            hash_table_for_each(object->u.table->table, hiter) {
                variable = hash_table_iter_element(hiter, table_pair_t, link);

                if (__heap_value_is_object__(variable->key)) {
                    __heap_mark_object__(variable->key->u.object_value);
                }

                if (__heap_value_is_object__(variable->value)) {
                    __heap_mark_object__(variable->value->u.object_value);
                }
            }

            hash_table_iter_free(hiter);
        }
        break;

    case OBJECT_TYPE_ARRAY:
        array_for_each(obj->u.array, base, index) {
            if (__heap_value_is_object__(base[index])) {
                __heap_mark_object__(base[index]->u.object_value);
            }
        }
        break;

    case OBJECT_TYPE_TABLE:
        hiter = hash_table_iter_new(obj->u.table->table);
        hash_table_for_each(obj->u.table->table, hiter) {
            variable = hash_table_iter_element(hiter, table_pair_t, link);

            if (__heap_value_is_object__(variable->key)) {
                __heap_mark_object__(variable->key->u.object_value);
            }

            if (__heap_value_is_object__(variable->value)) {
                __heap_mark_object__(variable->value->u.object_value);
            }
        }

        hash_table_iter_free(hiter);
        break;

    default:
        break;
    }
}