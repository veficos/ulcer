

#ifndef _ULCER_STMT_H_
#define _ULCER_STMT_H_

#include "config.h"
#include "expr.h"
#include "list.h"
#include "array.h"
#include "hlist.h"
#include "cstring.h"
#include "hash_table.h"

typedef enum stmt_type_e     stmt_type_t;
typedef struct stmt_s*       stmt_t;
typedef struct stmt_if_s     stmt_if_t;
typedef struct stmt_elif_s   stmt_elif_t;
typedef struct stmt_while_s  stmt_while_t;
typedef struct stmt_for_s    stmt_for_t;

enum stmt_type_e {
    STMT_TYPE_EXPR,
    STMT_TYPE_BREAK,
    STMT_TYPE_CONTINUE,
    STMT_TYPE_RETURN,
    STMT_TYPE_BLOCK,
    STMT_TYPE_IF,
    STMT_TYPE_ELIF,
    STMT_TYPE_WHILE,
    STMT_TYPE_FOR,
};

struct stmt_elif_s {
    expr_t condition;
    stmt_t block;
};

struct stmt_if_s {
    expr_t condition;
    stmt_t if_block;
    list_t elifs;
    stmt_t else_block;
};

struct stmt_while_s {
    expr_t condition;
    stmt_t block;
};

struct stmt_for_s {
    expr_t init;
    expr_t condition;
    expr_t post;
    stmt_t block;
};

struct stmt_s {
    stmt_type_t type;
    long        line;
    long        column;
    union {
        expr_t        expr;
        list_t        block;
        expr_t        return_expr;
        stmt_if_t     stmt_if;
        stmt_elif_t   stmt_elif;
        stmt_while_t  stmt_while;
        stmt_for_t    stmt_for;
    }u;
    list_node_t link;
};

stmt_t stmt_new_expr(long line, long column, expr_t expr);
stmt_t stmt_new_break(long line, long column);
stmt_t stmt_new_continue(long line, long column);
stmt_t stmt_new_return(long line, long column, expr_t return_value);
stmt_t stmt_new_if(long line, long column, expr_t condition, stmt_t if_block);
stmt_t stmt_new_elif(long line, long column, expr_t condition, stmt_t block);
stmt_t stmt_new_while(long line, long column, expr_t condition, stmt_t block);
stmt_t stmt_new_for(long line, long column, expr_t init, expr_t condition, expr_t post, stmt_t block);
stmt_t stmt_new_block(long line, long column);
void stmt_free(stmt_t stmt);

typedef struct parameter_s*  parameter_t;
typedef struct function_s*   function_t;
typedef enum function_type_e function_type_t;

struct parameter_s {
    cstring_t   name;
    list_node_t link;
};

enum function_type_e {
    FUNCTION_TYPE_USER,
    FUNCTION_TYPE_NATIVE,
};

typedef array_t (*native_function_pt)(array_t args);

struct function_s {
    function_type_t type;
    long            line;
    long            column;

    cstring_t       name;
    union {
        struct {
            native_function_pt function;
        }native;
        struct {
            list_t parameters;
            stmt_t block;
        }user;
    }u;
    hlist_node_t link;
};

parameter_t parameter_new(cstring_t name);
void parameter_free(parameter_t parameter);

function_t function_new_user(long line, long column, cstring_t name);
function_t function_new_native(cstring_t name, native_function_pt function);
void function_free(function_t func);

struct closure_s {
    long       line;
    long       column;

    cstring_t  name;
    list_t     parameters;
    stmt_t     block;

    hlist_node_t link;

    list_t init_args;
    bool   init_call;
};

closure_t closure_new(long line, long column, cstring_t name);
void closure_free(closure_t closure);

#endif