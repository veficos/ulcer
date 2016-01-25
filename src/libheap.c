

#include "native.h"
#include "error.h"
#include "evaluator.h"
#include "environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void native_heap_alloc(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    environment_push_pointer(env, (void*)malloc(native_check_int_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_heap_free(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    free(native_check_pointer_value(values[0]));

    environment_pop_value(env);

    environment_push_null(env);
}

void import_heap_library(environment_t env)
{
    struct pair_s {
        char* name;
        native_function_pt func;
    };

    int i;
    value_t string_table;

    environment_push_str(env, "heap");

    environment_push_table(env);

    string_table = list_element(list_rbegin(env->stack), value_t, link);

    table_push_pair(environment_get_global_table(env), env);

    struct pair_s pairs[] = {
        { "alloc",           native_heap_alloc  },
        { "free",            native_heap_free   },
    };

    for (i = 0; i < sizeof(pairs) / sizeof(struct pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_native_function(env, pairs[i].func);
        table_push_pair(string_table->u.object_value->u.table, env);
    }
}