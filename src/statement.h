

#ifndef _ULCER_STATEMENT_H_
#define _ULCER_STATEMENT_H_

#include "config.h"
#include "list.h"
#include "hlist.h"
#include "stack.h"
#include "cstring.h"
#include "hash_table.h"
#include "expression.h"

typedef enum statement_type_e           statement_type_t;
typedef struct statement_s*             statement_t;
typedef struct statement_if_s*          statement_if_t;
typedef struct statement_elif_s*        statement_elif_t;
typedef struct statement_switch_s*      statement_switch_t;
typedef struct statement_switch_case_s* statement_switch_case_t;
typedef struct statement_while_s*       statement_while_t;
typedef struct statement_for_s*         statement_for_t;

enum statement_type_e {
    STATEMENT_TYPE_REQUIRE,
    STATEMENT_TYPE_EXPRESSION,
    STATEMENT_TYPE_IF,
    STATEMENT_TYPE_SWITCH,
    STATEMENT_TYPE_WHILE,
    STATEMENT_TYPE_FOR,
    STATEMENT_TYPE_CONTINUE,
    STATEMENT_TYPE_BREAK,
    STATEMENT_TYPE_RETURN,
};

struct statement_if_s {
    expression_t condition;
    list_t       if_block;
    list_t       elifs;
    list_t       else_block;
};

struct statement_elif_s {
    expression_t condition;
    list_t       block;
    list_node_t  link;
};

struct statement_switch_s {
    expression_t expr;
    list_t       cases;
    list_t       default_block;
};

struct statement_switch_case_s {
    expression_t case_expr;
    list_t       block;
    list_node_t  link;
};

struct statement_while_s {
    expression_t condition;
    list_t       block;
};

struct statement_for_s {
    expression_t init;
    expression_t condition;
    expression_t post;
    list_t       block;
};

struct statement_s {
    statement_type_t type;
    long             line;
    long             column;

    union {
        cstring_t          package_name;
        expression_t       expr;
        expression_t       return_expr;
        statement_if_t     if_stmt;
        statement_switch_t switch_stmt;
        statement_while_t  while_stmt;
        statement_for_t    for_stmt;
    }u;

    list_node_t  link;
};

statement_elif_t        statement_new_elif(expression_t condition, list_t block);
void                    statement_free_elif(statement_elif_t elif);
statement_switch_case_t statement_new_switch_case(expression_t case_expr, list_t block);
void                    statement_free_switch_case(statement_switch_case_t switch_case);

statement_t statement_new_require(long line, long column, cstring_t package_name);
statement_t statement_new_expression(long line, long column, expression_t expr);
statement_t statement_new_if(long line, long column, expression_t condition, list_t if_block, list_t elifs, list_t else_block);
statement_t statement_new_switch(long line, long column, expression_t switch_expr, list_t cases, list_t default_block);
statement_t statement_new_while(long line, long column, expression_t condition, list_t block);
statement_t statement_new_for(long line, long column, expression_t init, expression_t condition, expression_t post, list_t block);
statement_t statement_new_continue(long line, long column);
statement_t statement_new_break(long line, long column);
statement_t statement_new_return(long line, long column, expression_t return_expr);

void statement_free(statement_t stmt);

#endif