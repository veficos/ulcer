

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
    value_t value;
    
    value = value_new(src->type);
    
    *value = *src;

    switch (value->type) {
    case VALUE_TYPE_FUNCTION:
    case VALUE_TYPE_NATIVE_FUNCTION:
        break;
    }
    
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

    if (l->key->type == r->key->type) {
        switch (l->key->type) {
        case VALUE_TYPE_NULL:
            return 0;
        case VALUE_TYPE_CHAR:
            return l->key->u.char_value - r->key->u.char_value;
        case VALUE_TYPE_BOOL:
            return l->key->u.bool_value - r->key->u.bool_value;
        case VALUE_TYPE_INT:
            return l->key->u.int_value - r->key->u.int_value;
        case VALUE_TYPE_LONG:
            return l->key->u.long_value - r->key->u.long_value;
        case VALUE_TYPE_FLOAT:
            return (int)(l->key->u.float_value - r->key->u.float_value);
        case VALUE_TYPE_DOUBLE:
            return (int)(l->key->u.double_value - r->key->u.double_value);
        case VALUE_TYPE_NATIVE_FUNCTION:
            return (int)(l->key->u.object_value->u.function - r->key->u.object_value->u.function);
        case VALUE_TYPE_FUNCTION:
            return (int)(l->key->u.object_value->u.function - r->key->u.object_value->u.function);
        case VALUE_TYPE_STRING:
            return cstring_cmp(l->key->u.object_value->u.string, r->key->u.object_value->u.string);
        case VALUE_TYPE_ARRAY:
            return (int)(l->key->u.object_value->u.array - r->key->u.object_value->u.array);
        case VALUE_TYPE_TABLE:
            return (int)(l->key->u.object_value->u.table - r->key->u.object_value->u.table);
        case VALUE_TYPE_POINTER:
            return (int)((uintptr_t)l->key->u.pointer_value - (uintptr_t)r->key->u.pointer_value);
        }
    }

    return l->key->type - r->key->type;
}

static unsigned long __table_key_hashfn__(const hlist_node_t *hnode)
{
    table_pair_t pair = hlist_element(hnode, table_pair_t, link);

    switch (pair->key->type) {
    case VALUE_TYPE_NULL:
        return 2;
    case VALUE_TYPE_CHAR:
        return pair->key->u.char_value;
    case VALUE_TYPE_BOOL:
        return pair->key->u.bool_value;
    case VALUE_TYPE_INT:
        return golden_ratio_prime_hash_32(pair->key->u.int_value, 32);
    case VALUE_TYPE_LONG:
        return (unsigned long)golden_ratio_prime_hash_ptr((uintptr_t)pair->key->u.long_value);
    case VALUE_TYPE_FLOAT:
        return (unsigned long)golden_ratio_prime_hash_ptr((uintptr_t)pair->key->u.float_value);
    case VALUE_TYPE_DOUBLE:
        return (unsigned long)golden_ratio_prime_hash_ptr((uintptr_t)pair->key->u.double_value);
    case VALUE_TYPE_NATIVE_FUNCTION:
    case VALUE_TYPE_FUNCTION:
        return (unsigned long)golden_ratio_prime_hash_ptr((uintptr_t)pair->key->u.object_value->u.function);
    case VALUE_TYPE_STRING:
        return (unsigned long)murmur2_hash((unsigned char*)pair->key->u.object_value->u.string, cstring_length(pair->key->u.object_value->u.string));
    case VALUE_TYPE_ARRAY:
        return (unsigned long)golden_ratio_prime_hash_ptr((uintptr_t)pair->key->u.object_value->u.array);
    case VALUE_TYPE_TABLE:
        return (unsigned long)golden_ratio_prime_hash_ptr((uintptr_t)pair->key->u.object_value->u.table);
    case VALUE_TYPE_POINTER:
        return (unsigned long)golden_ratio_prime_hash_ptr((uintptr_t)pair->key->u.pointer_value);
    }

    return 0ul;
}

static void __table_node_destructor__(hlist_node_t *node)
{
    table_pair_t pair = hlist_element(node, table_pair_t, link);

    value_free(pair->key);

    value_free(pair->value);

    mem_free(pair);
}

static hlist_node_ops_t __table_hash_operators__ = {
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

    table->table = hash_table_new(&__table_hash_operators__);

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

void table_push_pair(table_t table, environment_t env)
{
    table_pair_t pair;
    value_t k, v;

    assert(!list_is_empty(env->stack));

    v = list_element(list_rbegin(env->stack), value_t, link);

    list_pop_back(env->stack);

    assert(!list_is_empty(env->stack));

    k = list_element(list_rbegin(env->stack), value_t, link);

    list_pop_back(env->stack);

    pair = (table_pair_t) mem_alloc(sizeof(struct table_pair_s));

    pair->key = k;
    pair->value = v;

    hash_table_replace(table->table, &pair->link);
}

void table_add_member(table_t table, value_t key, value_t value)
{
    table_pair_t pair = (table_pair_t) mem_alloc(sizeof(struct table_pair_s));
    
    pair->key   = key;
    pair->value = value; 

    hash_table_replace(table->table, &pair->link);
}

value_t table_search(table_t table, environment_t env)
{
    struct table_pair_s pair;
    hlist_node_t* node;

    pair.key = list_element(list_rbegin(env->stack), value_t, link);

    node = hash_table_search(table->table, &pair.link);
    if (!node) {
        return NULL;
    }

    return hlist_element(node, table_pair_t, link)->value;
}

value_t table_search_by_value(table_t table, value_t key)
{
    struct table_pair_s pair;
    hlist_node_t* node;

    pair.key = key;

    node = hash_table_search(table->table, &pair.link);
    if (!node) {
        return NULL;
    }

    return hlist_element(node, table_pair_t, link)->value;
}

value_t table_new_member(table_t table, value_t key)
{
    table_pair_t pair;
    value_t value;

    pair = (table_pair_t) mem_alloc(sizeof(struct table_pair_s));
    if (!pair) {
        return NULL;
    }

    value = value_new(VALUE_TYPE_NULL);

    pair->key   = key;
    pair->value = value;

    hash_table_replace(table->table, &pair->link);

    return value;
}

static int __environment_package_key_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs)
{
    package_t l = hlist_element(lhs, package_t, link);
    package_t r = hlist_element(rhs, package_t, link);
    return cstring_cmp(l->name, r->name);
}

static unsigned long __environment_package_key_hashfn__(const hlist_node_t *hnode)
{
    package_t package = hlist_element(hnode, package_t, link);
    return (unsigned long)murmur2_hash((unsigned char*)package->name, cstring_length(package->name));
}

static void __environment_package_node_destructor__(hlist_node_t *node)
{
    package_t package = hlist_element(node, package_t, link);

    cstring_free(package->name);

    mem_free(package);
}

static hlist_node_ops_t __environment_package_operators__ = {
    NULL,
    &__environment_package_node_destructor__,
    &__environment_package_key_hashfn__,
    &__environment_package_key_compare__,
    NULL,
};

environment_t environment_new(void)
{
    environment_t env = (environment_t) mem_alloc(sizeof(struct environment_s));

    env->global_table = table_new();
    env->heap         = heap_new();
    env->packages      = hash_table_new(&__environment_package_operators__);

    list_init(env->stack);
    list_init(env->modules);
    list_init(env->local_context_stack);

    stack_init(env->statement_stack);

    return env;
}

void environment_free(environment_t env)
{
    list_iter_t iter, next_iter;

    table_clear(env->global_table);

    list_init(env->local_context_stack);

    heap_gc(env);

    table_free(env->global_table);

    heap_free(env->heap);
    
    hash_table_free(env->packages);

    list_safe_for_each(env->modules, iter, next_iter) {
        list_erase(env->modules, *iter);
        module_free(list_element(iter, module_t, link));
    }

    mem_free(env);
}

void environment_add_module(environment_t env, module_t module)
{
    list_iter_t iter;

    list_for_each(module->functions, iter) {
        statement_t stmt = list_element(iter, statement_t, link);
        assert(stmt->type == STATEMENT_TYPE_EXPRESSION && stmt->u.expr->type == EXPRESSION_TYPE_FUNCTION);
        if (!cstring_is_empty(stmt->u.expr->u.function_expr->name)) {
            environment_push_string(env, stmt->u.expr->u.function_expr->name);

            environment_push_function(env, stmt->u.expr->u.function_expr);

            table_push_pair(environment_get_global_table(env), env);
        }
    }

    stack_push(env->statement_stack, module->statements->link);

    list_push_back(env->modules, module->link);
}

bool environment_has_package(environment_t env, cstring_t name)
{
    struct package_s package;
    hlist_node_t* node;

    package.name = name;

    node = hash_table_search(env->packages, &package.link);
    if (!node) {
        return false;
    }

    return true;
}

void environment_add_package(environment_t env, cstring_t name)
{
    package_t package;

    package = (package_t) mem_alloc(sizeof(struct package_s));

    package->name = name;

    hash_table_replace(env->packages, &package->link);
}

table_t environment_get_global_table(environment_t env)
{
    return env->global_table;
}

void environment_push_local_context(environment_t env)
{
    local_context_t context;

    context = (local_context_t)mem_alloc(sizeof(struct local_context_s));

    context->object = heap_alloc_table(env);

    list_push_back(env->local_context_stack, context->link);
}

void environment_push_scope_local_context(environment_t env, object_t object)
{
    local_context_t context;

    context = (local_context_t)mem_alloc(sizeof(struct local_context_s));

    context->object = object;

    list_push_back(env->local_context_stack, context->link);
}

void environment_pop_local_context(environment_t env)
{
    local_context_t context;

    context = list_element(list_rbegin(env->local_context_stack), local_context_t, link);

    list_pop_back(env->local_context_stack);

    mem_free(context);
}

void environment_clear_stack(environment_t env)
{
    list_iter_t iter, next_iter;
    list_safe_for_each(env->stack, iter, next_iter) {
        list_erase(env->stack, *iter);
        value_free(list_element(iter, value_t, link));
    }
}

void environment_xchg_stack(environment_t env)
{
    value_t v1, v2;
    
    assert(!list_is_empty(env->stack));

    v1 = list_element(list_rbegin(env->stack), value_t, link);

    list_pop_back(env->stack);

    assert(!list_is_empty(env->stack));

    v2 = list_element(list_rbegin(env->stack), value_t, link);

    list_pop_back(env->stack);

    list_push_back(env->stack, v1->link);
    list_push_back(env->stack, v2->link);
}

void environment_pop_value(environment_t env)
{
    value_t value;

    value = list_element(list_rbegin(env->stack), value_t, link);

    list_pop_back(env->stack);

    value_free(value);
}

void environment_push_pointer(environment_t env, void *pointer)
{
    value_t value = value_new(VALUE_TYPE_POINTER);

    value->u.pointer_value = pointer;

    list_push_back(env->stack, value->link);
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

void environment_push_str(environment_t env, const char* str)
{
    value_t value = value_new(VALUE_TYPE_STRING);

    value->u.object_value = heap_alloc_str(env, str);

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
        list_push_back(value->u.object_value->u.function->scopes, context->object->link_scope);
    }

    list_push_back(env->stack, value->link);
}

void environment_push_native_function(environment_t env, native_function_pt native_function)
{
    value_t value = value_new(VALUE_TYPE_NATIVE_FUNCTION);

    value->u.object_value = heap_alloc_native_function(env, native_function);

    list_push_back(env->stack, value->link);
}

void environment_push_array_generate(environment_t env, list_t array_generate)
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

        evaluator_expression(env, expr);

        elem = list_element(list_rbegin(env->stack), value_t, link);

        list_pop_back(env->stack);

        dst = (value_t*) array_push(value->u.object_value->u.array);

        *dst = elem;
    }
}

void environment_push_array(environment_t env)
{
    value_t array_value;

    array_value = value_new(VALUE_TYPE_ARRAY);

    array_value->u.object_value = heap_alloc_array(env);

    list_push_back(env->stack, array_value->link);
}

void environment_push_table_generate(environment_t env, list_t table_generate)
{
    list_iter_t     iter;
    value_t         table_value;
    value_t         value;

    table_value = value_new(VALUE_TYPE_TABLE);

    table_value->u.object_value = heap_alloc_table(env);

    list_push_back(env->stack, table_value->link);

    list_for_each(table_generate, iter) {
        expression_table_pair_t pair;

        pair = list_element(iter, expression_table_pair_t, link);

        evaluator_expression(env, pair->member_name);

        value = list_element(list_rbegin(env->stack), value_t, link);

        if (value->type == VALUE_TYPE_NULL && pair->member_name->type == EXPRESSION_TYPE_IDENTIFIER) {
            value->u.object_value = heap_alloc_string(env, pair->member_name->u.identifier_expr);
            value->type = VALUE_TYPE_STRING;
        }

        evaluator_expression(env, pair->member_expr);

        table_push_pair(table_value->u.object_value->u.table, env);
    }
}

void environment_push_table(environment_t env)
{
    value_t table_value;
   
    table_value = value_new(VALUE_TYPE_TABLE);

    table_value->u.object_value = heap_alloc_table(env);

    list_push_back(env->stack, table_value->link);
}