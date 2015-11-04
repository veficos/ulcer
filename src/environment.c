

#include "environment.h"
#include "alloc.h"
#include "heap.h"

environment_t environment_new(void)
{
    environment_t env = (environment_t) mem_alloc(sizeof(struct environment_s));
    if (!env) {
        return NULL;
    }

    env->heap  = heap_new();
    env->stack = array_new(sizeof(struct value_s));

    return env;
}

void environment_free(environment_t env)
{
    heap_free(env->heap);
    array_free(env->stack);
    mem_free(env);
}

void environment_add_module(environment_t env, module_t module)
{
    env->module = module;
}