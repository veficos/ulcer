

#ifndef _ULCER_EXPR_H_
#define _ULCER_EXPR_H_

#include "config.h"
#include "list.h"
#include "token.h"
#include "cstring.h"

typedef enum expr_type_e             expr_type_t;
typedef struct expr_s*               expr_t;
typedef struct expr_call_s           expr_call_t;
typedef struct expr_assign_s         expr_assign_t;
typedef struct expr_binary_s         expr_binary_t;
typedef struct expr_array_index_s    expr_array_index_t;
typedef struct expr_array_generate_s expr_array_generate_t;
typedef struct closure_s*            closure_t;

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
    EXPR_TYPE_ADD_ASSIGN,
    EXPR_TYPE_SUB_ASSIGN,
    EXPR_TYPE_MUL_ASSIGN,
    EXPR_TYPE_DIV_ASSIGN,
    EXPR_TYPE_MOD_ASSIGN,
    EXPR_TYPE_CALL,
    EXPR_TYPE_CLOSURE,
    EXPR_TYPE_PLUS,
    EXPR_TYPE_MINUS,
    EXPR_TYPE_FLIP,
    EXPR_TYPE_NOT,
    EXPR_TYPE_INC,
    EXPR_TYPE_DEC,
    EXPR_TYPE_BITAND,
    EXPR_TYPE_BITOR,
    EXPR_TYPE_XOR,
    EXPR_TYPE_LEFT_SHIFT,
    EXPR_TYPE_RIGHT_SHIFT,
    EXPR_TYPE_LOGIC_RIGHT_SHIFT,
    EXPR_TYPE_MUL,
    EXPR_TYPE_DIV,
    EXPR_TYPE_MOD,
    EXPR_TYPE_ADD,
    EXPR_TYPE_SUB,
    EXPR_TYPE_GT,
    EXPR_TYPE_GEQ,
    EXPR_TYPE_LT,
    EXPR_TYPE_LEQ,
    EXPR_TYPE_EQ,
    EXPR_TYPE_NEQ,
    EXPR_TYPE_AND,
    EXPR_TYPE_OR,
    EXPR_TYPE_ARRAY_INDEX,
    EXPR_TYPE_ARRAY_GENERATE,
};

struct expr_assign_s {
    expr_t lvalue;
    expr_t rvalue;
};

struct expr_call_s {
    cstring_t name;
    list_t    args;
};

struct expr_binary_s {
    expr_t left;
    expr_t right;
};

struct expr_array_index_s {
    expr_t array;
    expr_t index;
};

struct expr_array_generate_s {
    list_t exprs;
};

struct expr_s {
    expr_type_t type;

    long        line;
    long        column;

    union {
        char                  char_value;
        bool                  bool_value;
        int                   int_value;
        long                  long_value;
        float                 float_value;
        double                double_value;
        cstring_t             string_value;
        cstring_t             identifier;
        expr_assign_t         assign;
        expr_call_t           call;
        expr_t                unary;
        expr_binary_t         binary;
        expr_array_index_t    array_index;
        expr_array_generate_t array_generate;
        closure_t             closure;
    }u;

    list_node_t link;
};

const char* expr_type_string(expr_type_t expr_type);

expr_t expr_new(expr_type_t type, token_t tok);
expr_t expr_new_assign(long line, long column, expr_t lvalue, expr_t rvalue);
expr_t expr_new_add_assign(long line, long column, expr_t lvalue, expr_t rvalue);
expr_t expr_new_sub_assign(long line, long column, expr_t lvalue, expr_t rvalue);
expr_t expr_new_mul_assign(long line, long column, expr_t lvalue, expr_t rvalue);
expr_t expr_new_div_assign(long line, long column, expr_t lvalue, expr_t rvalue);
expr_t expr_new_mod_assign(long line, long column, expr_t lvalue, expr_t rvalue);
expr_t expr_new_identifier(long line, long column, cstring_t identifier);
expr_t expr_new_call(long line, long column, cstring_t identifier);
expr_t expr_new_flip(long line, long column, expr_t exp);
expr_t expr_new_not(long line, long column, expr_t exp);
expr_t expr_new_inc(long line, long column, expr_t exp);
expr_t expr_new_dec(long line, long column, expr_t exp);
expr_t expr_new_plus(long line, long column, expr_t exp);
expr_t expr_new_minus(long line, long column, expr_t exp);
expr_t expr_new_binary(expr_type_t type, long line, long column, expr_t left, expr_t right);
expr_t expr_new_array_index(long line, long column, expr_t array, expr_t index);
expr_t expr_new_array_generate(long line, long column);
expr_t expr_new_closure(long line, long column, closure_t closure);
void expr_free(expr_t expr);

#endif