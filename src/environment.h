

#ifndef _ULCER_ENVIRONMENT_H_
#define _ULCER_ENVIRONMENT_H_

#include "config.h"
#include "list.h"
#include "stmt.h"
#include "hlist.h"
#include "array.h"
#include "module.h"
#include "cstring.h"
#include "hash_table.h"

typedef enum object_type_e        object_type_t;
typedef struct object_s*          object_t;
typedef enum value_type_e         value_type_t;
typedef struct value_s*           value_t;
typedef struct heap_s*            heap_t;
typedef struct local_context_s*   local_context_t;
typedef struct variable_s*        variable_t;
typedef struct environment_s*     environment_t;

enum object_type_e {
    OBJECT_TYPE_STRING,
};

struct object_s {
    object_type_t type;
    bool marked;

    union {
        cstring_t string;
    }u;

    list_node_t link;
};

enum value_type_e {
    VALUE_TYPE_CHAR,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_LONG,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_STRING,
    VALUE_TYPE_CLOSURE,
    VALUE_TYPE_POINTER,
    VALUE_TYPE_REFERENCE,
    VALUE_TYPE_NULL,
};

struct value_s {
    value_type_t type;
    union {
        char      char_value;
        bool      bool_value;
        int       int_value;
        long      long_value;
        float     float_value;
        double    double_value;
        object_t  object_value;
        void*     pointer_value;
        closure_t closure_value;
    }u;
};

const char* value_type_string(value_t value);

struct variable_s {
    cstring_t    name;
    value_t      value;
    hlist_node_t link;
};

struct local_context_s {
    hash_table_t variables;
    list_node_t  link;
};

struct environment_s {
    module_t module;
    array_t  stack;
    heap_t   heap;
    hash_table_t global_context;
    list_t       local_context_stack;
};

environment_t environment_new(void);
void environment_free(environment_t env);
void environment_add_module(environment_t env, module_t module);

function_t environment_search_function(environment_t env, cstring_t function_name);

void environment_push_local_context(environment_t env);
void environment_pop_local_context(environment_t env);
void environment_new_local_variable_by_lvalue_expr(environment_t env, expr_t lvalue_expr, value_t rvalue);
void environment_new_local_variable_by_identifier(environment_t env, cstring_t varname, value_t rvalue);
value_t environment_search_local_lvalue_variable_by_lvalue_expr(environment_t env, expr_t lvalue_expr);
value_t environment_search_local_lvalue_variable_by_identifier(environment_t env, cstring_t varname);

void environment_new_global_lvalue_variable_by_value_expr(environment_t env, expr_t lvalue_expr, value_t rvalue);
value_t environment_search_global_lvalue_variable(environment_t env, expr_t lvalue_expr);

#endif