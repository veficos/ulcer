
#include "native.h"
#include "error.h"
#include "evaluator.h"
#include "environment.h"
#include "heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void native_runtime_gc(environment_t env, unsigned int argc)
{
    heap_gc(env);

    environment_pop_value(env);

    environment_push_null(env);
}

void import_runtime_library(environment_t env)
{
    struct pair_s {
        char* name;
        native_function_pt func;
    };

    int i;
    value_t runtime_table;

    environment_push_str(env, "runtime");

    environment_push_table(env);

    runtime_table = list_element(list_rbegin(env->stack), value_t, link);

    table_push_pair(environment_get_global_table(env), env);
   
    struct pair_s pairs[] = {
        { "gc",         native_runtime_gc },
    };

    for (i = 0; i < sizeof(pairs) / sizeof(struct pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_native_function(env, pairs[i].func);
        table_push_pair(runtime_table->u.object_value->u.table, env);
    }
}