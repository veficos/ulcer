

#include "native.h"
#include "error.h"
#include "evaluator.h"
#include "environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

static void native_math_sqrt(environment_t env, unsigned int argc)
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

    environment_push_double(env, sqrt(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_sin(environment_t env, unsigned int argc)
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

    environment_push_double(env, sin(native_check_double_value(values[0])));

    environment_xchg_stack(env);
    
    environment_pop_value(env);
}

static void native_math_cos(environment_t env, unsigned int argc)
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

    environment_push_double(env, cos(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_tan(environment_t env, unsigned int argc)
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

    environment_push_double(env, tan(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_asin(environment_t env, unsigned int argc)
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

    environment_push_double(env, asin(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_acos(environment_t env, unsigned int argc)
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

    environment_push_double(env, acos(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_atan(environment_t env, unsigned int argc)
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

    environment_push_double(env, atan(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_atan2(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t *);

    if (argc < 2) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    environment_push_double(env, atan2(
        native_check_double_value(values[0]),
        native_check_double_value(values[1])
        ));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_sinh(environment_t env, unsigned int argc)
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

    environment_push_double(env, sinh(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_cosh(environment_t env, unsigned int argc)
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

    environment_push_double(env, cosh(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_math_tanh(environment_t env, unsigned int argc)
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

    environment_push_double(env, tanh(native_check_double_value(values[0])));

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void import_math_const(environment_t env, table_t table)
{
    struct const_double_pair_s {
        char* name;
        double i;
    };

    int i;

    struct const_double_pair_s pairs[] = {
        { "PI",        3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172 },
    };

    for (i = 0; i < sizeof(pairs) / sizeof(struct const_double_pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_double(env, pairs[i].i);
        table_push_pair(table, env);
    }
}

void import_math_library(environment_t env)
{
    struct pair_s {
        char* name;
        native_function_pt func;
    };

    int i;
    value_t math_table;

    environment_push_str(env, "math");

    environment_push_table(env);

    math_table = list_element(list_rbegin(env->stack), value_t, link);

    table_push_pair(environment_get_global_table(env), env);

    import_math_const(env, math_table->u.object_value->u.table);

    struct pair_s pairs[] = {
        { "sqrt",         native_math_sqrt },
        { "sin",          native_math_sin },
        { "cos",          native_math_cos },
        { "tan",          native_math_tan },
        { "asin",         native_math_asin },
        { "acos",         native_math_acos },
        { "atan",         native_math_atan },
        { "atan2",        native_math_atan2 },
        { "cosh",         native_math_cosh },
    };

    for (i = 0; i < sizeof(pairs) / sizeof(struct pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_native_function(env, pairs[i].func);
        table_push_pair(math_table->u.object_value->u.table, env);
    }
}