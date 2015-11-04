

#ifndef _ULCER_ENVIRONMENT_H_
#define _ULCER_ENVIRONMENT_H_

#include "config.h"
#include "list.h"
#include "array.h"
#include "module.h"
#include "cstring.h"

typedef enum object_type_e    object_type_t;
typedef struct object_s*      object_t;
typedef enum value_type_e     value_type_t;
typedef struct value_s*       value_t;
typedef struct heap_s*        heap_t;
typedef struct environment_s* environment_t;

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
    VALUE_TYPE_RESERVE,
    VALUE_TYPE_CHAR,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_LONG,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_STRING,
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
    }u;
};

struct variable_s {
    cstring_t name;
    value_t   value;
};

struct global_variable_s {
    list_node_t link;
};

struct environment_s {
    module_t module;
    array_t  stack;
    heap_t   heap;
};

environment_t environment_new(void);
void environment_free(environment_t env);
void environment_add_module(environment_t env, module_t module);
void environment_add_native_function(environment_t env);

#endif