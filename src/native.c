
#include "native.h"
#include "error.h"
#include "evaluator.h"
#include "environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void setup_native_module(environment_t env)
{
#ifdef USE_LIBNATIVE
#   include "libnative.h"
    import_native_library(env);
#endif

#ifdef USE_LIBSTR
#   include "libstr.h"
    import_string_library(env);
#endif

#ifdef USE_LIBMATH
#   include "libmath.h"
    import_math_library(env);
#endif

#ifdef USE_LIBSDL
#   include "libsdl.h"
    import_libsdl_library(env);
#endif

#ifdef USE_LIBHEAP
#   include "libheap.h"
    import_heap_library(env);
#endif

#ifdef USE_LIBFILE
#   include "libfile.h"
    import_file_library(env);
#endif

#ifdef USE_LIBRUNTIME
#   include "libruntime.h"
    import_runtime_library(env);
#endif
}

void* native_check_pointer_value(value_t value)
{
    if (value->type == VALUE_TYPE_NULL) {
        return NULL;
    } else if (value->type == VALUE_TYPE_POINTER) {
        return value->u.pointer_value;
    } else {
        runtime_error("passing '%s' to parameter of incompatible type 'pointer'",
            get_value_type_string(value->type));
    }

    return NULL;
}

int native_check_int_value(value_t value)
{
    if (value->type == VALUE_TYPE_INT) {
        return value->u.int_value;
    } else {
        runtime_error("passing '%s' to parameter of incompatible type 'int'",
            get_value_type_string(value->type));
    }

    return 0;
}

const char* native_check_string_value(value_t value)
{
    if (value->type == VALUE_TYPE_STRING) {
        return value->u.object_value->u.string;
    } else {
        runtime_error("passing '%s' to parameter of incompatible type 'string'",
            get_value_type_string(value->type));
    }

    return NULL;
}

bool native_check_bool_value(value_t value)
{
    if (value->type == VALUE_TYPE_BOOL) {
        return value->u.bool_value;
    } else {
        runtime_error("passing '%s' to parameter of incompatible type 'bool'",
            get_value_type_string(value->type));
    }
    
    return false;
}

double native_check_double_value(value_t value)
{
    switch (value->type) {
    case VALUE_TYPE_CHAR:
        return (double)value->u.char_value;
    case VALUE_TYPE_INT:
        return (double)value->u.int_value;
    case VALUE_TYPE_LONG:
        return (double)value->u.long_value;
    case VALUE_TYPE_FLOAT:
        return (double)value->u.float_value;
    case VALUE_TYPE_DOUBLE:
        return (double)value->u.double_value;
    }

    return 0.0;
}