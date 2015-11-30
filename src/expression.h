

#ifndef _ULCER_EXPRESSION_H_
#define _ULCER_EXPRESSION_H_

#include "config.h"
#include "list.h"
#include "token.h"
#include "cstring.h"

typedef enum   expression_type_e                expression_type_t;
typedef struct expression_s*                    expression_t;
typedef struct expression_function_parameter_s* expression_function_parameter_t;
typedef struct expression_function_s*           expression_function_t;
typedef struct expression_call_s*               expression_call_t;
typedef struct expression_assign_s*             expression_assign_t;
typedef struct expression_component_assign_s*   expression_component_assign_t;
typedef struct expression_binary_s*             expression_binary_t;

typedef struct expression_table_pair_s*         expression_table_pair_t;
typedef struct expression_table_dot_member_s*   expression_table_dot_member_t;
typedef struct expression_array_push_s*         expression_array_push_t;
typedef struct expression_array_pop_s*          expression_array_pop_t;
typedef struct expression_index_s*              expression_index_t;

enum expression_type_e {

    /* value expression */
    EXPRESSION_TYPE_CHAR,
    EXPRESSION_TYPE_BOOL,
    EXPRESSION_TYPE_INT,
    EXPRESSION_TYPE_LONG,
    EXPRESSION_TYPE_FLOAT,
    EXPRESSION_TYPE_DOUBLE,
    EXPRESSION_TYPE_STRING,
    EXPRESSION_TYPE_NULL,
    EXPRESSION_TYPE_FUNCTION,

    EXPRESSION_TYPE_IDENTIFIER,
    EXPRESSION_TYPE_IGNORE,

    EXPRESSION_TYPE_LIST,

    /* assign expression */
    EXPRESSION_TYPE_ASSIGN,
    EXPRESSION_TYPE_ADD_ASSIGN,
    EXPRESSION_TYPE_SUB_ASSIGN,
    EXPRESSION_TYPE_MUL_ASSIGN,
    EXPRESSION_TYPE_DIV_ASSIGN,
    EXPRESSION_TYPE_MOD_ASSIGN,
    EXPRESSION_TYPE_BITAND_ASSIGN,
    EXPRESSION_TYPE_BITOR_ASSIGN,
    EXPRESSION_TYPE_XOR_ASSIGN,
    EXPRESSION_TYPE_LEFI_SHIFT_ASSIGN,
    EXPRESSION_TYPE_RIGHT_SHIFT_ASSIGN,
    EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT_ASSIGN,

    EXPRESSION_TYPE_CALL,

    EXPRESSION_TYPE_PLUS,
    EXPRESSION_TYPE_MINUS,

    /* inc dec expression */
    EXPRESSION_TYPE_INC,
    EXPRESSION_TYPE_DEC,

    /* bitop expression */
    EXPRESSION_TYPE_CPL,
    EXPRESSION_TYPE_BITAND,
    EXPRESSION_TYPE_BITOR,
    EXPRESSION_TYPE_XOR,
    EXPRESSION_TYPE_LEFT_SHIFT,
    EXPRESSION_TYPE_RIGHT_SHIFT,
    EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT,

    /* math expression */
    EXPRESSION_TYPE_MUL,
    EXPRESSION_TYPE_DIV,
    EXPRESSION_TYPE_MOD,
    EXPRESSION_TYPE_ADD,
    EXPRESSION_TYPE_SUB,
    
    /* logic expression */
    EXPRESSION_TYPE_GT,
    EXPRESSION_TYPE_GEQ,
    EXPRESSION_TYPE_LT,
    EXPRESSION_TYPE_LEQ,
    EXPRESSION_TYPE_EQ,
    EXPRESSION_TYPE_NEQ,
    EXPRESSION_TYPE_AND,
    EXPRESSION_TYPE_OR,
    EXPRESSION_TYPE_NOT,

    EXPRESSION_TYPE_ARRAY_GENERATE,
    EXPRESSION_TYPE_TABLE_GENERATE,
    EXPRESSION_TYPE_ARRAY_PUSH,
    EXPRESSION_TYPE_ARRAY_POP,
    EXPRESSION_TYPE_TABLE_DOT_MEMBER,

    EXPRESSION_TYPE_INDEX,
};

struct expression_function_parameter_s {
    cstring_t   name;
    list_node_t link;
};

struct expression_function_s {
    cstring_t name;
    list_t    parameters;
    list_t    block;
};

struct expression_call_s {
    expression_t function_expr;
    list_t       args;
};

struct expression_assign_s {
    list_t lvalue_exprs;
    list_t rvalue_exprs;
    int    lvalue_exprs_total;
    int    rvalue_exprs_total;
};

struct expression_component_assign_s {
    expression_t lvalue_expr;
    expression_t rvalue_expr;
};

struct expression_binary_s {
    expression_t left;
    expression_t right;
};

struct expression_array_push_s {
    expression_t array_expr;
    expression_t elem_expr;
};

struct expression_array_pop_s {
    expression_t array_expr;
    expression_t lvalue_expr;
};

struct expression_table_pair_s {
    cstring_t    member_name;
    expression_t member_expr;
    list_node_t  link;
};

struct expression_table_dot_member_s {
    expression_t table;
    cstring_t    member_name;
};

struct expression_index_s {
    expression_t dict;
    expression_t index;
};

struct expression_s {
    expression_type_t type;
    long              line;
    long              column;

    union {
        char                            char_expr;
        bool                            bool_expr;
        int                             int_expr;
        long                            long_expr;
        float                           float_expr;
        double                          double_expr;
        cstring_t                       string_expr;
        expression_function_t           function_expr;
        list_t                          array_generate_expr;
        list_t                          table_generate_expr;
        expression_index_t              index_expr;
        expression_array_push_t         array_push_expr;
        expression_array_pop_t          array_pop_expr;
        expression_table_dot_member_t   table_dot_member_expr;
        cstring_t                       identifier_expr;
        expression_binary_t             binary_expr;
        expression_t                    unary_expr;
        expression_call_t               call_expr;
        expression_assign_t             assign_expr;
        expression_component_assign_t   component_assign_expr;
        expression_t                    incdec_expr;
        list_t                          expressions_expr;
    }u;

    list_node_t link;
};

const char*             expression_type_string(expression_type_t expr_type);
expression_t            expression_new_literal(expression_type_t type, token_t tok);
expression_t            expression_new_identifier(long line, long column, cstring_t identifier);
expression_t            expression_new_ignore(long line, long column);
expression_t            expression_new_assign(long line, long column, list_t lvalue_exprs, int lvalue_exprs_total,  list_t rvalue_exprs, int rvalue_exprs_total);
expression_t            expression_new_component_assign(long line, long column, expression_type_t component_assign_type, expression_t lvalue_expr, expression_t rvalue_expr);
expression_t            expression_new_binary(long line, long column, expression_type_t binary_expr_type, expression_t left, expression_t right);
expression_t            expression_new_unary(long line, long column, expression_type_t unary_expr_type, expression_t expression);
expression_t            expression_new_incdec(long line, long column, expression_type_t type, expression_t lvalue_expr);
expression_t            expression_new_function(long line, long column, cstring_t name, list_t parameters, list_t block);
expression_t            expression_new_call(long line, long column, expression_t function_expr, list_t args);
expression_t            expression_new_list(long line, long column, list_t exprs);
expression_t            expression_new_array_generate(long line, long column, list_t elements);
expression_t            expression_new_array_push(long line, long column, expression_t array_expr, expression_t elem_expr);
expression_t            expression_new_array_pop(long line, long column, expression_t array_expr, expression_t lvalue_expr);
expression_table_pair_t expression_new_table_pair(cstring_t name, expression_t expr);
void                    expression_free_table_pair(expression_table_pair_t pair);
expression_t            expression_new_table_generate(long line, long column, list_t members);
expression_t            expression_new_table_dot_member(long line, long column, expression_t table, cstring_t member_name);
expression_t            expression_new_index(long line, long column, expression_t dict, expression_t index);
void                    expression_free(expression_t expr);

#endif