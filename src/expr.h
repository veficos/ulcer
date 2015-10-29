

#ifndef _ULCER_EXPR_H_
#define _ULCER_EXPR_H_

#include "config.h"
#include "list.h"
#include "token.h"
#include "cstring.h"

typedef enum expr_type_e expr_type_t;
typedef struct expr_s* expr_t;
typedef struct expr_call_s expr_call_t;
typedef struct expr_assign_s expr_assign_t;

enum expr_type_e {
    EXPR_TYPE_CHAR,
    EXPR_TYPE_BOOL,
    EXPR_TYPE_INT,
    EXPR_TYPE_LONG,
    EXPR_TYPE_FLOAT,
    EXPR_TYPE_DOUBLE,
    EXPR_TYPE_STRING,
    EXPR_TYPE_NULL,
    EXPR_TYPE_IDENTIFIER,
    EXPR_TYPE_ASSIGN,
    EXPR_TYPE_CALL,
    EXPR_TYPE_PLUS,
    EXPR_TYPE_MINUS,
};

struct expr_assign_s {
    cstring_t lvalue;
    expr_t    rvalue;
};

struct expr_call_s {
    cstring_t function_name;
    list_t    args;
};

struct expr_binary_s {
    expr_t left;
    expr_t right;
};

typedef struct expr_s {
    expr_type_t type;
    long        line;
    long        column;
    union {
        char      char_value;
        bool      bool_value;
        int       int_value;
        long      long_value;
        float     float_value;
        double    double_value;
        cstring_t string_value;
        cstring_t identifier;
        expr_assign_t assign;
        expr_call_t   call;
        expr_t        minus;
        expr_t        plus;
    }u;

    list_node_t link;
}* expr_t;


expr_t expr_new(expr_type_t type, token_t tok);
expr_t expr_new_assign(long line, long column, cstring_t lvalue, expr_t rvalue);
expr_t expr_new_identifier(long line, long column, cstring_t identifier);
expr_t expr_new_call(long line, long column, cstring_t identifier);
expr_t expr_new_plus(long line, long column, expr_t expr);
expr_t expr_new_minus(long line, long column, expr_t expr);

#endif