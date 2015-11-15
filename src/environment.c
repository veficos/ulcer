

#include "environment.h"
#include "hash_table.h"
#include "hashfn.h"
#include "alloc.h"
#include "heap.h"

#include <assert.h>

static int __environment_variable_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs);
static unsigned long __environment_variable_hashfn__(const hlist_node_t *hnode);
static void __environment_free_global_variable__(environment_t env);
static void __environment_free_local_variables__(hash_table_t htable);

static hlist_node_ops_t variable_hash_operators = {
    NULL,
    NULL,
    &__environment_variable_hashfn__,
    &__environment_variable_compare__,
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
    case VALUE_TYPE_REFERENCE:
        return "null";
    case VALUE_TYPE_NULL:
        return "null";
    }

    return "unknown";
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
    env->global_context = hash_table_new(&variable_hash_operators);

    list_init(env->local_context_stack);

    return env;
}

void environment_free(environment_t env)
{
    __environment_free_global_variable__(env);
    while (!list_is_empty(env->local_context_stack)) {
        environment_pop_local_context(env);
    }
    array_free(env->stack);
    heap_free(env->heap);
    mem_free(env);
}

void environment_add_module(environment_t env, module_t module)
{
    env->module = module;
}

function_t environment_search_function(environment_t env, cstring_t function_name)
{
    return module_search_function(env->module, function_name);
}

void environment_push_local_context(environment_t env)
{
    local_context_t lctx = (local_context_t) 
        mem_alloc(sizeof(struct local_context_s));

    lctx->variables  = hash_table_new(&variable_hash_operators);
  
    list_push_back(env->local_context_stack, lctx->link);
}

void environment_pop_local_context(environment_t env)
{
    local_context_t lctx;

    assert(!list_is_empty(env->local_context_stack));

    lctx = list_element(list_rbegin(env->local_context_stack), local_context_t, link);

    list_pop_back(env->local_context_stack);

    __environment_free_local_variables__(lctx->variables);
 
    mem_free(lctx);
}

void environment_new_local_variable_by_lvalue_expr(environment_t env, expr_t lvalue_expr, value_t rvalue)
{
    local_context_t lctx;
    variable_t variable;
    value_t v;

    assert(!list_is_empty(env->local_context_stack));

    lctx = list_element(list_rbegin(env->local_context_stack), local_context_t, link);

    variable = (variable_t) mem_alloc(sizeof(struct variable_s));
    v = (value_t) mem_alloc(sizeof(struct value_s));
    *v = *rvalue;

    if (lvalue_expr->type == EXPR_TYPE_IDENTIFIER) {
        variable->name  = cstring_dup(lvalue_expr->u.identifier);
        variable->value = v;
    }

    hash_table_insert(lctx->variables, &variable->link);
}

void environment_new_local_variable_by_identifier(environment_t env, cstring_t varname, value_t rvalue)
{
    local_context_t lctx;
    variable_t variable;
    value_t v;

    assert(!list_is_empty(env->local_context_stack));

    lctx = list_element(list_rbegin(env->local_context_stack), local_context_t, link);

    variable = (variable_t)mem_alloc(sizeof(struct variable_s));
    v = (value_t)mem_alloc(sizeof(struct value_s));
    *v = *rvalue;
  
    variable->name = cstring_dup(varname);
    variable->value = v;

    hash_table_insert(lctx->variables, &variable->link);
}

value_t environment_search_local_lvalue_variable_by_lvalue_expr(environment_t env, expr_t lvalue_expr)
{
    local_context_t lctx;
    list_iter_t iter;
    struct variable_s variable;
    hlist_node_t *node = NULL;
    value_t result = NULL;
    bool is_reference = false;
    
    if (list_is_empty(env->local_context_stack)) {
        return result;
    }

    if (lvalue_expr->type == EXPR_TYPE_IDENTIFIER) {
        variable.name = lvalue_expr->u.identifier;
    }

    list_reverse_for_each(env->local_context_stack, iter) {
        lctx = list_element(iter, local_context_t, link);

        node = hash_table_search(lctx->variables, &variable.link);
        if (node) {
            break;
        }
    }

    if (node) {
        result = hlist_element(node, variable_t, link)->value;
    }

    return result;
}

value_t environment_search_local_lvalue_variable_by_identifier(environment_t env, cstring_t varname)
{
    local_context_t lctx;
    list_iter_t iter;
    struct variable_s variable;
    hlist_node_t *node = NULL;
    value_t result = NULL;
    bool is_reference = false;

    if (list_is_empty(env->local_context_stack)) {
        return result;
    }
   
    variable.name = varname;

    list_reverse_for_each(env->local_context_stack, iter) {
        lctx = list_element(iter, local_context_t, link);

        node = hash_table_search(lctx->variables, &variable.link);
        if (node) {
            break;
        }
    }

    if (node) {
        result = hlist_element(node, variable_t, link)->value;
    }

    return result;
}

void environment_new_global_lvalue_variable_by_value_expr(environment_t env, expr_t lvalue_expr, value_t rvalue)
{
    variable_t variable;
    value_t v;

    variable = (variable_t) mem_alloc(sizeof(struct variable_s));
    v = (value_t) mem_alloc(sizeof(struct value_s));

    *v = *rvalue;

    if (lvalue_expr->type == EXPR_TYPE_IDENTIFIER) {
        variable->name  = cstring_dup(lvalue_expr->u.identifier);
        variable->value = v;
    }

    hash_table_insert(env->global_context, &variable->link);
}

value_t environment_search_global_lvalue_variable(environment_t env, expr_t lvalue_expr)
{
    struct variable_s variable;
    hlist_node_t *node;

    if (lvalue_expr->type == EXPR_TYPE_IDENTIFIER) {
        variable.name = lvalue_expr->u.identifier;
    }

    node = hash_table_search(env->global_context, &variable.link);
    if (!node) {
        return NULL;
    }

    return hlist_element(node, variable_t, link)->value;
}

static int __environment_variable_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs)
{ 
    variable_t lv  = hlist_element(lhs, variable_t, link);
    variable_t rv = hlist_element(rhs, variable_t, link);
    return cstring_cmp(lv->name, rv->name);
}

static unsigned long __environment_variable_hashfn__(const hlist_node_t *hnode)
{
    variable_t var = hlist_element(hnode, variable_t, link);
    return (unsigned long) 
        murmur2_hash(var->name, cstring_length(var->name));
}

static void __environment_free_global_variable__(environment_t env)
{
    hash_table_iter_t iter;
    variable_t variable;

    iter = hash_table_iter_new(env->global_context);
    hash_table_for_each(env->global_context, iter) {
        hlist_remove(iter.hn);
        variable = hash_table_iter_element(iter, variable_t, link);
        
        cstring_free(variable->name);
        mem_free(variable->value);
        mem_free(variable);
    }

    hash_table_iter_free(iter);

    hash_table_free(env->global_context);
}

static void __environment_free_local_variables__(hash_table_t htable)
{
    hash_table_iter_t iter;
    variable_t variable;

    iter = hash_table_iter_new(htable);
    hash_table_for_each(htable, iter) {
        hlist_remove(iter.hn);
        variable = hash_table_iter_element(iter, variable_t, link);

        cstring_free(variable->name);
        mem_free(variable->value);
        mem_free(variable);
    }

    hash_table_iter_free(iter);

    hash_table_free(htable);
}