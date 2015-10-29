

#include "expr.h"
#include "alloc.h"

#include <stdio.h>

expr_t expr_new(expr_type_t type, token_t tok)
{
    expr_t expr = heap_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = type;
    expr->line   = tok->line;
    expr->column = tok->column;

    switch (type) {
    case EXPR_TYPE_BOOL:
        if (tok->value == TOKEN_VALUE_TRUE) {
            expr->u.bool_value = true;
        } else if (tok->value == TOKEN_VALUE_FALSE) {
            expr->u.bool_value = false;
        }
        break;

    case EXPR_TYPE_CHAR:
        sscanf(tok->token, "%c", &expr->u.char_value);
        break;

    case EXPR_TYPE_INT:
        sscanf(tok->token, "%d", &expr->u.int_value);
        break;

    case EXPR_TYPE_LONG:
        sscanf(tok->token, "%ld", &expr->u.long_value);
        break;

    case EXPR_TYPE_FLOAT:
        sscanf(tok->token, "%f", &expr->u.float_value);
        break;

    case EXPR_TYPE_DOUBLE:
        sscanf(tok->token, "%lf", &expr->u.double_value);
        break;

    case EXPR_TYPE_STRING:
        expr->u.string_value = cstring_dup(tok->token);
        break;
    }

    return expr;
}

expr_t expr_new_identifier(long line, long column, cstring_t identifier)
{
    expr_t expr = heap_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_ASSIGN;
    expr->line   = line;
    expr->column = column;
    expr->u.identifier = identifier;

    return expr;
}

expr_t expr_new_assign(long line, long column, cstring_t lvalue, expr_t rvalue)
{
    expr_t expr = heap_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_ASSIGN;
    expr->line   = line;
    expr->column = column;
    expr->u.assign.lvalue = lvalue;
    expr->u.assign.rvalue = rvalue;

    return expr;
}

expr_t expr_new_call(long line, long column, cstring_t identifier)
{
    expr_t expr = heap_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_CALL;
    expr->line   = line;
    expr->column = column;
    expr->u.call.function_name = identifier;
    list_init(expr->u.call.args);

    return expr;
}