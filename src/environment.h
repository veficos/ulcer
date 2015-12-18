

#ifndef _ULCER_ENVIRONMENT_H_
#define _ULCER_ENVIRONMENT_H_

#include "config.h"
#include "stack.h"
#include "array.h"
#include "hlist.h"
#include "list.h"
#include "cstring.h"
#include "expression.h"
#include "module.h"

typedef struct environment_s*   environment_t;
typedef struct function_s*      function_t;
typedef enum object_type_e      object_type_t;
typedef struct object_s*        object_t;
typedef enum value_type_e       value_type_t;
typedef struct value_s*         value_t;
typedef struct table_pair_s*    table_pair_t;
typedef struct table_s*         table_t;
typedef struct local_context_s* local_context_t;

enum object_type_e {
    OBJECT_TYPE_STRING,
    OBJECT_TYPE_ARRAY,
    OBJECT_TYPE_TABLE,
    OBJECT_TYPE_NATIVE_FUNCTION,
    OBJECT_TYPE_FUNCTION,
};

typedef void (*native_function_pt)(environment_t env, unsigned int argc);

struct function_s {
    union {
        expression_function_t function_expr;
        native_function_pt    native_function;
    } f;
    list_t scopes;
};

struct object_s {
    object_type_t type;
    bool marked;

    union {       
        cstring_t       string;
        array_t         array;
        hash_table_t    table;
        function_t      function;
    } u;

    list_node_t link;
};

enum value_type_e {
    VALUE_TYPE_NIL,

    VALUE_TYPE_CHAR,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_LONG,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_NATIVE_FUNCTION,
    VALUE_TYPE_FUNCTION,
    VALUE_TYPE_STRING,
    VALUE_TYPE_ARRAY,
    VALUE_TYPE_TABLE,

    VALUE_TYPE_POINTER,
    VALUE_TYPE_REFERENCE,
    VALUE_TYPE_NULL,
};

struct value_s {
    value_type_t type;

    union {
        char                  char_value;
        bool                  bool_value;
        int                   int_value;
        long                  long_value;
        float                 float_value;
        double                double_value;
        object_t              object_value;
        void*                 pointer_value;
    }u;

    list_node_t link;
};

value_t value_new(value_type_t type);
value_t value_dup(const value_t src);
void    value_free(value_t value);

struct table_pair_s {
    cstring_t    key;
    value_t      value;
    hlist_node_t link;
};

struct table_s {
    hash_table_t table;
};

table_t table_new(void);
void    table_free(table_t table);
value_t table_search_member(table_t table, cstring_t member_name);
value_t table_new_member(table_t table, cstring_t member_name);
void    table_add_member(table_t table, cstring_t key, value_t value);

typedef struct heap_s* heap_t;

struct local_context_s {
    table_t  context;
    list_node_t link;
};

struct environment_s {
    stack_t statement_stack;
    list_t  local_context_stack;
    list_t  stack;
    heap_t  heap;
    table_t global_table;

    list_t  function_stack;
};

environment_t environment_new(void);
void          environment_free(environment_t env);
void          environment_add_module(environment_t env, module_t module);
table_t       environment_get_global_table(environment_t env);

/* local context */
void          environment_push_local_context(environment_t env);
void          environment_push_scope_local_context(environment_t env, local_context_t context);
void          environment_pop_local_context(environment_t env);

/* stack op */
void          environment_clear_stack(environment_t env);
void          environment_push_value(environment_t env, value_t value);
void          environment_push_char(environment_t env, char char_value);
void          environment_push_bool(environment_t env, bool bool_value);
void          environment_push_int(environment_t env, int int_value);
void          environment_push_long(environment_t env, long long_value);
void          environment_push_float(environment_t env, float float_value);
void          environment_push_double(environment_t env, double double_value);
void          environment_push_string(environment_t env, cstring_t string_value);
void          environment_push_null(environment_t env);
void          environment_push_function(environment_t env, expression_function_t function);
void          environment_push_native_function(environment_t env, native_function_pt native_function);
void          environment_push_array_generate(environment_t env, list_t array_generate, bool toplevel);

/* function stack */
void          environment_push_value_to_function_stack(environment_t env, value_t v);
void          environment_pop_value_from_function_stack(environment_t env);

#endif