

#ifndef _ULCER_EXECUTOR_H_
#define _ULCER_EXECUTOR_H_

#include "config.h"
#include "list.h"
#include "array.h"
#include "function.h"

typedef enum object_type_e object_type_t;
typedef struct object_s*   object_t;
typedef enum value_type_e  value_type_t;
typedef struct value_s     value_t;
typedef struct heap_s      heap_t;
typedef struct executor_s* executor_t;

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

struct heap_s {
    long current_heap_size;
    long current_heap_threshold;
    list_t objects;
};

struct executor_s {
    list_t      statements;
    functions_t functions;
    array_t     stack;
    heap_t      heap;
};

executor_t executor_new(list_t statements, functions_t functions);
void executor_free(executor_t exec);
void executor_run(executor_t exec);

#endif