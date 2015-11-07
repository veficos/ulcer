

#include "environment.h"
#include "hash_table.h"
#include "hashfn.h"
#include "alloc.h"
#include "heap.h"

static int __environment_global_variable_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs);
static unsigned long __environment_global_variable_hashfn__(const hlist_node_t *hnode);

static hlist_node_ops_t global_variable_operators = {
    NULL,
    NULL,
    &__environment_global_variable_hashfn__,
    &__environment_global_variable_compare__,
    NULL,
};

environment_t environment_new(void)
{
    environment_t env = (environment_t) 
        mem_alloc(sizeof(struct environment_s));
    if (!env) {
        return NULL;
    }

    env->heap  = heap_new();
    env->stack = array_new(sizeof(struct value_s));
    env->global_context = hash_table_new(&global_variable_operators);

    return env;
}

void environment_free(environment_t env)
{
    array_free(env->stack);

    heap_free(env->heap);
    mem_free(env);
}

void environment_add_module(environment_t env, module_t module)
{
    env->module = module;
}

static int __environment_global_variable_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs)
{ 
    global_variable_t lv  = hlist_element(lhs, global_variable_t, link);
    global_variable_t rv = hlist_element(rhs, global_variable_t, link);
    return cstring_cmp(lv->name, rv->name);
}

static unsigned long __environment_global_variable_hashfn__(const hlist_node_t *hnode)
{
    global_variable_t var = hlist_element(hnode, global_variable_t, link);
    return (unsigned long) 
        murmur2_hash(var->name, cstring_length(var->name));
}