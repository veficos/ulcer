

#ifndef _ULCER_FUNCTION_H_
#define _ULCER_FUNCTION_H_

#include "config.h"
#include "hash_table.h"
#include "cstring.h"
#include "hlist.h"
#include "list.h"
#include "stmt.h"

typedef struct parameter_s* parameter_t;
typedef struct function_s*  function_t;
typedef struct functions_s* functions_t;

struct parameter_s {
    cstring_t   name;
    list_node_t link;
};

struct function_s {
    cstring_t name;
    long      line;
    long      column;
    union {
        struct {
            list_t parameters;
            stmt_t block;
        }self;
    }u;
    hlist_node_t link;
};

struct functions_s {
    hash_table_t     htable;
    hlist_node_ops_t ops;
};

parameter_t parameter_new(cstring_t name);
void parameter_free(parameter_t parameter);

function_t function_new(long line, long column, cstring_t name);
void function_free(function_t function);

functions_t function_library_new();
bool functions_add(functions_t fl, function_t function);
bool functions_del(functions_t fl, const char* function_name);
void functions_free(functions_t fl);

#endif