

#include "environment.h"
#include "hash_table.h"
#include "hashfn.h"
#include "hlist.h"
#include "alloc.h"
#include "heap.h"
#include "evaluator.h"

#include <assert.h>

value_t value_new(value_type_t type)
{
    value_t value = (value_t) mem_alloc(sizeof(struct value_s));
    if (!value) {
        return NULL;
    }

    value->type = type;

    return value;
}

value_t value_dup(const value_t src)
{
    value_t value = value_new(src->type);
    *value = *src;
    return value;
}

void value_free(value_t value)
{
    mem_free(value);
}

static int __table_key_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs)
{
    table_pair_t l = hlist_element(lhs, table_pair_t, link);
    table_pair_t r = hlist_element(rhs, table_pair_t, link);
    return cstring_cmp(l->key, r->key);
}

static unsigned long __table_key_hashfn__(const hlist_node_t *hnode)
{
    table_pair_t pair = hlist_element(hnode, table_pair_t, link);
    return murmur2_hash((unsigned char*)pair->key, cstring_length(pair->key));
}

static void __table_node_destructor__(hlist_node_t *node)
{
    table_pair_t pair = hlist_element(node, table_pair_t, link);

    cstring_free(pair->key);
    value_free(pair->value);
    mem_free(pair);
}

static hlist_node_ops_t table_hash_operators = {
    NULL,
    &__table_node_destructor__,
    &__table_key_hashfn__,
    &__table_key_compare__,
    NULL,
};

table_t table_new(void)
{
    table_t table = (table_t) mem_alloc(sizeof(struct table_s));
    if (!table) {
        return NULL;
    }

    table->table = hash_table_new(&table_hash_operators);

    return table;
}

void table_free(table_t table)
{
    hash_table_free(table->table);
    mem_free(table);
}

void table_clear(table_t table)
{
    hash_table_clear(table->table);
}

void table_add_member(table_t table, cstring_t key, value_t value)
{
    table_pair_t pair = (table_pair_t) mem_alloc(sizeof(struct table_pair_s));
    
    pair->key   = key;
    pair->value = value; 

    hash_table_replace(table->table, &pair->link);
}

value_t table_search_member(table_t table, cstring_t member_name)
{
    struct table_pair_s pair;
    hlist_node_t* node;

    pair.key = member_name;

    node = hash_table_search(table->table, &pair.link);
    if (!node) {
        return NULL;
    }

    return hlist_element(node, table_pair_t, link)->value;
}

value_t table_new_member(table_t table, cstring_t member_name)
{
    table_pair_t pair;
    value_t value;

    pair = (table_pair_t) mem_alloc(sizeof(struct table_pair_s));
    if (!pair) {
        return NULL;
    }

    value = value_new(VALUE_TYPE_REFERENCE);

    pair->key   = cstring_dup(member_name);
    pair->value = value;

    hash_table_replace(table->table, &pair->link);

    return value;
}

environment_t environment_new(void)
{
    environment_t env = (environment_t) mem_alloc(sizeof(struct environment_s));

    env->global_table = table_new();
    env->heap         = heap_new();
    
    list_init(env->function_stack);
    list_init(env->stack);
    list_init(env->local_context_stack);

    stack_init(env->statement_stack);

    return env;
}

void environment_free(environment_t env)
{
    table_clear(env->global_table);

    heap_gc(env);

    table_free(env->global_table);

    heap_free(env->heap);

    mem_free(env);
}

void environment_add_module(environment_t env, module_t module)
{
    list_iter_t iter;

    list_for_each(module->functions, iter) {
        statement_t stmt = list_element(iter, statement_t, link);
        assert(stmt->type == STATEMENT_TYPE_EXPRESSION && stmt->u.expr->type == EXPRESSION_TYPE_FUNCTION);
        if (!cstring_is_empty(stmt->u.expr->u.function_expr->name)) {
            value_t value         = value_new(VALUE_TYPE_FUNCTION);
            value->u.object_value = heap_alloc_function(env, stmt->u.expr->u.function_expr);
            table_add_member(env->global_table, cstring_dup(stmt->u.expr->u.function_expr->name), value);
        }
    }

    stack_push(env->statement_stack, module->statements->link);
}

table_t environment_get_global_table(environment_t env)
{
    return env->global_table;
}

void environment_push_local_context(environment_t env)
{
    
}

void environment_push_scope_local_context(environment_t env, local_context_t context)
{
    
}

void environment_pop_local_context(environment_t env)
{
    
}

void environment_clear_stack(environment_t env)
{
    list_iter_t iter, next_iter;
    list_safe_for_each(env->stack, iter, next_iter) {
        list_erase(env->stack, *iter);
        value_free(list_element(iter, value_t, link));
    }
}

void environment_push_value(environment_t env, value_t value)
{
    list_push_back(env->stack, value->link);
}

void environment_push_char(environment_t env, char char_value)
{
    value_t value = value_new(VALUE_TYPE_CHAR);

    value->u.char_value = char_value;

    list_push_back(env->stack, value->link);
}

void environment_push_bool(environment_t env, bool bool_value)
{
    value_t value = value_new(VALUE_TYPE_BOOL);

    value->u.bool_value = bool_value;

    list_push_back(env->stack, value->link);
}

void environment_push_int(environment_t env, int int_value)
{
    value_t value = value_new(VALUE_TYPE_INT);

    value->u.int_value  = int_value;

    list_push_back(env->stack, value->link);
}

void environment_push_long(environment_t env, long long_value)
{
    value_t value = value_new(VALUE_TYPE_LONG);

    value->u.long_value = long_value;

    list_push_back(env->stack, value->link);
}

void environment_push_float(environment_t env, float float_value)
{
    value_t value = value_new(VALUE_TYPE_FLOAT);

    value->u.float_value = float_value;

    list_push_back(env->stack, value->link);
}

void environment_push_double(environment_t env, double double_value)
{
    value_t value = value_new(VALUE_TYPE_DOUBLE);

    value->u.double_value = double_value;

    list_push_back(env->stack, value->link);
}

void environment_push_string(environment_t env, cstring_t string_value)
{
    value_t value = value_new(VALUE_TYPE_STRING);

    value->u.object_value = heap_alloc_string(env, string_value);

    list_push_back(env->stack, value->link);
}

void environment_push_null(environment_t env)
{
    value_t value = value_new(VALUE_TYPE_NULL);

    list_push_back(env->stack, value->link);
}

void environment_push_function(environment_t env, expression_function_t function_expr)
{
    value_t value;
    list_iter_t iter;
    local_context_t context;
    
    value = value_new(VALUE_TYPE_FUNCTION);

    value->u.object_value = heap_alloc_function(env, function_expr);

    list_for_each(env->local_context_stack, iter) {
        context = list_element(iter, local_context_t, link);
        //list_push_back(value->u.object_value->u.function->scopes, context->link.scope);
    }

    list_push_back(env->stack, value->link);
}

void environment_push_native_function(environment_t env, native_function_pt native_function)
{
    value_t value = value_new(VALUE_TYPE_NATIVE_FUNCTION);

    value->u.object_value = heap_alloc_native_function(env, native_function);

    list_push_back(env->stack, value->link);
}

void environment_push_array_generate(environment_t env, list_t array_generate, bool toplevel)
{
    list_iter_t  iter;
    expression_t expr;
    value_t      value;
    value_t      elem;
    value_t*     dst;

    value = value_new(VALUE_TYPE_ARRAY);

    value->u.object_value = heap_alloc_array_n(env, 10);

    list_push_back(env->stack, value->link);

    list_for_each(array_generate, iter) {
        expr = list_element(iter, expression_t, link);

        evaluator_expression(env, expr, toplevel);

        elem = list_element(list_rbegin(env->stack), value_t, link);

        list_pop_back(env->stack);

        dst = (value_t*) array_push(value->u.object_value->u.array);

        *dst = elem;
    }
}

void environment_push_value_to_function_stack(environment_t env, value_t v)
{
    list_push_back(env->function_stack, v->link);
}

void environment_pop_value_from_function_stack(environment_t env)
{
    if (list_is_empty(env->function_stack)) {
        return;
    }
    list_pop_back(env->function_stack);
}