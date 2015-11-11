

#include "environment.h"
#include "hash_table.h"
#include "hashfn.h"
#include "alloc.h"
#include "heap.h"

#include <assert.h>

static int __environment_global_variable_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs);
static unsigned long __environment_global_variable_hashfn__(const hlist_node_t *hnode);
static void __environment_free_global_variable__(environment_t env);

static hlist_node_ops_t global_variable_hash_operators = {
    NULL,
    NULL,
    &__environment_global_variable_hashfn__,
    &__environment_global_variable_compare__,
    NULL,
};

const char* value_type_string(value_t value)
{
    switch (value->type) {
    case VALUE_TYPE_CHAR:
        return "char";
    case VALUE_TYPE_BOOL:
        return "bool";
    case VALUE_TYPE_INT:
        return "int";
    case VALUE_TYPE_LONG:
        return "long";
    case VALUE_TYPE_FLOAT:
        return "float";
    case VALUE_TYPE_DOUBLE:
        return "double";
    case VALUE_TYPE_STRING:
        return "string";
    case VALUE_TYPE_NULL:
        return "null";
    }

    return "unkown";
}

environment_t environment_new(void)
{
    environment_t env = (environment_t) 
        mem_alloc(sizeof(struct environment_s));
    if (!env) {
        return NULL;
    }

    env->heap  = heap_new();
    env->stack = array_new(sizeof(struct value_s));
    env->global_context = hash_table_new(&global_variable_hash_operators);

    return env;
}

void environment_free(environment_t env)
{
    __environment_free_global_variable__(env);
    array_free(env->stack);
    heap_free(env->heap);
    mem_free(env);
}

void environment_add_module(environment_t env, module_t module)
{
    env->module = module;
}

void environment_push_local_context(environment_t env)
{
    local_context_t lctx = (local_context_t) 
        mem_alloc(sizeof(struct local_context_s));

    list_init(lctx->variables);

    list_init(lctx->references);

    list_push_back(env->local_context, lctx->link);
}

void environment_pop_local_context(environment_t env)
{
    local_context_t lctx;

    assert(!list_is_empty(env->local_context));

    lctx = list_element(list_rbegin(env->local_context), local_context_t, link);

    list_pop_back(env->local_context);

    mem_free(lctx);
}

void environment_new_local_variable(environment_t env, cstring_t name, value_t value)
{
    local_context_t lctx;
    local_variable_t variable;
    value_t v;

    assert(!list_is_empty(env->local_context));

    lctx = list_element(list_rbegin(env->local_context), local_context_t, link);

    variable = (local_variable_t) mem_alloc(sizeof(struct local_variable_s));
    v = (value_t) mem_alloc(sizeof(struct value_s));

    *v = *value;

    variable->name = cstring_dup(name);
    variable->value = v;

    list_push_back(lctx->variables, variable->link);
}

void environment_new_local_reference(environment_t env, cstring_t name, value_t value)
{
    local_context_t lctx;
    local_variable_t variable;
    value_t v;

    assert(!list_is_empty(env->local_context));

    lctx = list_element(list_rbegin(env->local_context), local_context_t, link);

    variable = (local_variable_t) mem_alloc(sizeof(struct local_variable_s));
    v = (value_t) mem_alloc(sizeof(struct value_s));

    *v = *value;

    variable->name = cstring_dup(name);
    variable->value = v;

    list_push_back(lctx->references, variable->link);
}

value_t environment_search_local_variable(environment_t env, cstring_t name)
{
    local_context_t lctx;
    local_variable_t variable;

    if (list_is_empty(env->local_context)) {
        return NULL;
    }

    lctx = list_element(list_rbegin(env->local_context), local_context_t, link);

    return NULL;
}

function_t environment_search_function(environment_t env, cstring_t function_name)
{
    return module_search_function(env->module, function_name);
}

void environment_new_global_variable(environment_t env, cstring_t name, value_t value)
{
    global_variable_t variable;
    value_t v;

    variable = (global_variable_t) mem_alloc(sizeof(struct global_variable_s));
    v = (value_t) mem_alloc(sizeof(struct value_s));

    *v = *value;

    variable->name  = cstring_dup(name);
    variable->value = v;

    hash_table_insert(env->global_context, &variable->link);
}

value_t environment_search_global_variable(environment_t env, cstring_t name)
{
    struct global_variable_s variable;
    hlist_node_t *node;

    variable.name = name;

    node = hash_table_search(env->global_context, &variable.link);
    if (!node) {
        return NULL;
    }

    return hlist_element(node, global_variable_t, link)->value;
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

static void __environment_free_global_variable__(environment_t env)
{
    hash_table_iter_t iter;
    global_variable_t variable;

    iter = hash_table_iter_new(env->global_context);
    hash_table_for_each(env->global_context, iter) {
        hlist_remove(iter.hn);
        variable = hash_table_iter_element(iter, global_variable_t, link);
        
        cstring_free(variable->name);
        mem_free(variable->value);
        mem_free(variable);
    }

    hash_table_iter_free(iter);

    hash_table_free(env->global_context);
}