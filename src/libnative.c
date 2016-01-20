#include "native.h"
#include "error.h"
#include "evaluator.h"
#include "environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void print_value(value_t value)
{
    switch (value->type) {
    case VALUE_TYPE_CHAR:
        printf("%c", value->u.char_value);
        break;

    case VALUE_TYPE_BOOL:
        value->u.bool_value == true ? printf("true") : printf("false");
        break;

    case VALUE_TYPE_INT:
        printf("%d", value->u.int_value);
        break;

    case VALUE_TYPE_LONG:
        printf("%ldl", value->u.long_value);
        break;

    case VALUE_TYPE_FLOAT:
        printf("%ff", value->u.float_value);
        break;

    case VALUE_TYPE_DOUBLE:
        printf("%lf", value->u.double_value);
        break;

    case VALUE_TYPE_STRING:
        printf("%s", value->u.object_value->u.string);
        break;

    case VALUE_TYPE_NULL:
        printf("null");
        break;

    case VALUE_TYPE_ARRAY:
    {
        int index;
        int last;
        value_t* base;

        last = array_length(value->u.object_value->u.array) - 1;

        printf("[");
        array_for_each(value->u.object_value->u.array, base, index) {
            print_value(base[index]);
            if (last != index) {
                printf(", ");
            }
        }
        printf("]");
    }
    break;

    case VALUE_TYPE_TABLE:
    {
        hash_table_iter_t hiter;
        table_pair_t pair;
        int index;
        int last;

        index = 0;
        last = hash_table_size(value->u.object_value->u.table->table) - 1;

        printf("{");
        hash_table_for_each(value->u.object_value->u.table->table, hiter) {
            pair = hash_table_iter_element(hiter, table_pair_t, link);

            print_value(pair->key);

            printf(":");

            print_value(pair->value);

            if (last != index++) {
                printf(", ");
            }
        }
        printf("}");
    }
    break;

    case VALUE_TYPE_NATIVE_FUNCTION:
    case VALUE_TYPE_FUNCTION:
        printf("(function, 0x%p)", value->u.object_value->u.function);
        break;

    case VALUE_TYPE_POINTER:
        printf("(pointer, 0x%p)", value->u.pointer_value);
        break;

    default:
        runtime_error("unknown type");
        break;
    }
}

static void native_print(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    int i;

    value = list_element(list_rbegin(env->stack), value_t, link);

    array_for_each(value->u.object_value->u.array, values, i) {
        print_value(values[i]);
    }

    environment_pop_value(env);
    environment_push_null(env);
}

static void native_type(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    if (argc == 0) {
        environment_push_null(env);
        return;
    }

    switch (values[0]->type) {
    case VALUE_TYPE_ARRAY:
        environment_push_str(env, "array");
        return;

    case VALUE_TYPE_TABLE:
        environment_push_str(env, "table");
        return;

    case VALUE_TYPE_STRING:
        environment_push_str(env, "string");
        return;

    case VALUE_TYPE_FUNCTION:
    case VALUE_TYPE_NATIVE_FUNCTION:
        environment_push_str(env, "function");
        return;

    case VALUE_TYPE_NULL:
        environment_push_str(env, "null");
        return;

    case VALUE_TYPE_CHAR:
        environment_push_str(env, "char");
        return;

    case VALUE_TYPE_BOOL:
        environment_push_str(env, "bool");
        return;

    case VALUE_TYPE_INT:
        environment_push_str(env, "int");
        return;

    case VALUE_TYPE_LONG:
        environment_push_str(env, "long");
        return;

    case VALUE_TYPE_FLOAT:
        environment_push_str(env, "float");
        return;

    case VALUE_TYPE_DOUBLE:
        environment_push_str(env, "double");
        return;

    case VALUE_TYPE_POINTER:
        environment_push_str(env, "pointer");
        return;

    default:
        runtime_error("unknown type");
        return;
    }
}

static void native_len(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    if (argc == 0) {
        environment_push_null(env);
        return;
    }

    switch (values[0]->type) {
    case VALUE_TYPE_ARRAY:
        environment_push_int(env, array_length(values[0]->u.object_value->u.array));
        return;
    case VALUE_TYPE_TABLE:
        environment_push_int(env, hash_table_size(values[0]->u.object_value->u.table->table));
        return;
    case VALUE_TYPE_STRING:
        environment_push_int(env, cstring_length(values[0]->u.object_value->u.string));
        return;
    default:
        environment_push_int(env, 0);
        return;
    }
}

void import_native_library(environment_t env)
{
    int i;
    table_t global_table;
    struct pair_s {
        char* name;
        native_function_pt func;
    } pairs[] = {
        { "print",          native_print },
        { "type",           native_type },
        { "len",            native_len },
    };

    global_table = environment_get_global_table(env);
    for (i = 0; i < sizeof(pairs) / sizeof(struct pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_native_function(env, pairs[i].func);
        table_push_pair(global_table, env);
    }
}
