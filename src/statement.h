

#ifndef _ULCER_STATEMENT_H_
#define _ULCER_STATEMENT_H_

#include "config.h"
#include "list.h"
#include "hlist.h"
#include "cstring.h"
#include "hash_table.h"
#include "expression.h"

typedef enum statement_type_e     statement_type_t;
typedef struct statement_s*       statement_t;
typedef struct statement_if_s     statement_if_t;
typedef struct statement_elif_s   statement_elif_t;
typedef struct statement_while_s  statement_while_t;
typedef struct statement_for_s    statement_for_t;

enum statement_type_e {
    STATEMENT_TYPE_REQUIRE,
    STATEMENT_TYPE_EXPRESSION,
    STATEMENT_TYPE_IF,
    STATEMENT_TYPE_ELIF,

    STATEMENT_TYPE_FOR,
    STATEMENT_TYPE_WHILE,

    STATEMENT_TYPE_BREAK,
    STATEMENT_TYPE_CONTINUE,
    STATEMENT_TYPE_RETURN,
};

struct statement_if_s {
    expression_t condition;
    list_t if_block;
    list_t elifs;
    list_t else_block;
};

struct statement_elif_s {
    expression_t condition;
    list_t block;
};

struct statement_switch_s {
    expression_t expr;
    list_t cases;
};

struct statement_while_s {
    expression_t condition;
    list_t block;
};

struct statement_for_s {
    expression_t init;
    expression_t condition;
    expression_t post;
    list_t block;
};

struct statement_s {
    statement_type_t type;
    long             line;
    long             column;

    union {
        cstring_t          package_name;
        expression_t       expr;
        expression_t       return_expr;
        statement_if_t     statement_if;
        statement_elif_t   statement_elif;
        statement_while_t  statement_while;
        statement_for_t    statement_for;
    }u;

    list_node_t link;
};

statement_t statement_new_require(long line, long column, cstring_t package_name);
statement_t statement_new_expression(long line, long column, expression_t expr);
statement_t statement_new_break(long line, long column);
statement_t statement_new_continue(long line, long column);
statement_t statement_new_return(long line, long column, expression_t return_value);
statement_t statement_new_if(long line, long column, expression_t condition, statement_t if_block);
statement_t statement_new_elif(long line, long column, expression_t condition, statement_t block);
statement_t statement_new_while(long line, long column, expression_t condition, statement_t block);
statement_t statement_new_for(long line, long column, expression_t init, expression_t condition, expression_t post, statement_t block);
statement_t statement_new_block(long line, long column);
void statement_free(statement_t stmt);

#endif