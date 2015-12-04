

#include "environment.h"
#include "hash_table.h"
#include "hashfn.h"
#include "hlist.h"
#include "alloc.h"

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
    value->u = src->u;
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

void table_add_member(table_t table, cstring_t key, value_t value)
{
    table_pair_t pair = (table_pair_t) mem_alloc(sizeof(struct table_pair_s));
    
    pair->key   = cstring_dup(key);
    pair->value = value_dup(value); 

    hash_table_replace(table->table, &pair->link);
}

void table_add_native_function(table_t table, cstring_t funcname, native_function_pt func)
{
    table_pair_t pair = (table_pair_t) mem_alloc(sizeof(struct table_pair_s));
    value_t value = value_new(VALUE_TYPE_NATIVE_FUNCTION);

    value->u.native_function_value = func;

    pair->key   = cstring_dup(funcname);
    pair->value = value; 

    hash_table_replace(table->table, &pair->link);
}

environment_t environment_new(void)
{
    environment_t env = (environment_t) mem_alloc(sizeof(struct environment_s));

    env->global_table = table_new();

    return env;
}

void environment_free(environment_t env)
{
    table_free(env->global_table);
    mem_free(env);
}

void environment_add_module(environment_t env, module_t module)
{
    list_iter_t iter;

    list_for_each(module->functions, iter) {
        statement_t stmt = list_element(iter, statement_t, llink);
        assert(stmt->type == STATEMENT_TYPE_EXPRESSION && stmt->u.expr->type == EXPRESSION_TYPE_FUNCTION);
        if (!cstring_is_empty(stmt->u.expr->u.function_expr->name)) {
            value_t value           = value_new(VALUE_TYPE_FUNCTION);
            value->u.function_value = stmt->u.expr->u.function_expr;
            table_add_member(env->global_table, stmt->u.expr->u.function_expr->name, value);
            value_free(value);
        }
    }

    stack_push(env->statement_stack, list_element(list_begin(module->statements), statement_t, llink)->slink);
}

table_t environment_get_global_table(environment_t env)
{
    return env->global_table;
}

void environment_add_native_function(environment_t env, cstring_t funcname, native_function_pt func)
{
    value_t value                   = value_new(VALUE_TYPE_NATIVE_FUNCTION);
    value->u.native_function_value  = func;

    table_add_member(env->global_table, funcname, value);

    value_free(value);
}