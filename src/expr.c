

#include "expr.h"
#include "stmt.h"
#include "alloc.h"

#include <stdio.h>
#include <assert.h>

const char* expr_type_string(expr_type_t expr_type)
{
    switch (expr_type) {
    case EXPR_TYPE_CHAR:
        return "char";
    case EXPR_TYPE_BOOL:
        return "bool";
    case EXPR_TYPE_INT:
        return "int";
    case EXPR_TYPE_LONG:
        return "long";
    case EXPR_TYPE_FLOAT:
        return "float";
    case EXPR_TYPE_DOUBLE:
        return "double";
    case EXPR_TYPE_STRING:
        return "string";
    case EXPR_TYPE_NULL:
        return "null";
    case EXPR_TYPE_IDENTIFIER:
        return "indentifier";
    case EXPR_TYPE_ASSIGN:
        return "=";
    case EXPR_TYPE_ADD_ASSIGN:
        return "+=";
    case EXPR_TYPE_SUB_ASSIGN:
        return "-=";
    case EXPR_TYPE_MUL_ASSIGN:
        return "*=";
    case EXPR_TYPE_DIV_ASSIGN:
        return "/=";
    case EXPR_TYPE_MOD_ASSIGN:
        return "%=";
    case EXPR_TYPE_CALL:
        return "function call";
    case EXPR_TYPE_PLUS:
        return "+";
    case EXPR_TYPE_MINUS:
        return "-";
    case EXPR_TYPE_NOT:
        return "!";
    case EXPR_TYPE_INC:
        return "++";
    case EXPR_TYPE_DEC:
        return "--";
    case EXPR_TYPE_BITAND:
        return "&";
    case EXPR_TYPE_BITOR:
        return "|";
    case EXPR_TYPE_XOR:
        return "^";
    case EXPR_TYPE_LEFT_SHIFT:
        return "<<";
    case EXPR_TYPE_RIGHT_SHIFT:
        return ">>";
    case EXPR_TYPE_LOGIC_RIGHT_SHIFT:
        return ">>>";
    case EXPR_TYPE_MUL:
        return "*";
    case EXPR_TYPE_DIV:
        return "/";
    case EXPR_TYPE_MOD:
        return "%";
    case EXPR_TYPE_ADD:
        return "+";
    case EXPR_TYPE_SUB:
        return "-";
    case EXPR_TYPE_GT:
        return ">";
    case EXPR_TYPE_GEQ:
        return ">=";
    case EXPR_TYPE_LT:
        return "<";
    case EXPR_TYPE_LEQ:
        return "<=";
    case EXPR_TYPE_EQ:
        return "==";
    case EXPR_TYPE_NEQ:
        return "!=";
    case EXPR_TYPE_AND:
        return "&&";
    case EXPR_TYPE_OR:
        return "||";
    }

    return "unknown";
}

expr_t expr_new(expr_type_t type, token_t tok)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
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
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_IDENTIFIER;
    expr->line   = line;
    expr->column = column;
    expr->u.identifier = identifier;

    return expr;
}

expr_t expr_new_assign(long line, long column, expr_t lvalue, expr_t rvalue)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
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

expr_t expr_new_add_assign(long line, long column, expr_t lvalue, expr_t rvalue)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_ADD_ASSIGN;
    expr->line   = line;
    expr->column = column;
    expr->u.assign.lvalue = lvalue;
    expr->u.assign.rvalue = rvalue;

    return expr;
}

expr_t expr_new_sub_assign(long line, long column, expr_t lvalue, expr_t rvalue)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_SUB_ASSIGN;
    expr->line   = line;
    expr->column = column;
    expr->u.assign.lvalue = lvalue;
    expr->u.assign.rvalue = rvalue;

    return expr;
}

expr_t expr_new_mul_assign(long line, long column, expr_t lvalue, expr_t rvalue)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_MUL_ASSIGN;
    expr->line   = line;
    expr->column = column;
    expr->u.assign.lvalue = lvalue;
    expr->u.assign.rvalue = rvalue;

    return expr;
}

expr_t expr_new_div_assign(long line, long column, expr_t lvalue, expr_t rvalue)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_DIV_ASSIGN;
    expr->line   = line;
    expr->column = column;
    expr->u.assign.lvalue = lvalue;
    expr->u.assign.rvalue = rvalue;

    return expr;
}

expr_t expr_new_mod_assign(long line, long column, expr_t lvalue, expr_t rvalue)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_MOD_ASSIGN;
    expr->line   = line;
    expr->column = column;
    expr->u.assign.lvalue = lvalue;
    expr->u.assign.rvalue = rvalue;

    return expr;
}

expr_t expr_new_call(long line, long column, cstring_t identifier)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type   = EXPR_TYPE_CALL;
    expr->line   = line;
    expr->column = column;
    expr->u.call.name = identifier;
    list_init(expr->u.call.args);

    return expr;
}

expr_t expr_new_plus(long line, long column, expr_t exp)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type = EXPR_TYPE_PLUS;
    expr->line = line;
    expr->column = column;
    expr->u.unary = exp;

    return expr;
}

expr_t expr_new_minus(long line, long column, expr_t exp)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type    = EXPR_TYPE_MINUS;
    expr->line    = line;
    expr->column  = column;
    expr->u.unary = exp;

    return expr;
}

expr_t expr_new_flip(long line, long column, expr_t exp)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type      = EXPR_TYPE_FLIP;
    expr->line      = line;
    expr->column    = column;
    expr->u.unary   = exp;

    return expr;
}

expr_t expr_new_not(long line, long column, expr_t exp)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type      = EXPR_TYPE_NOT;
    expr->line      = line;
    expr->column    = column;
    expr->u.unary   = exp;

    return expr;
}

expr_t expr_new_inc(long line, long column, expr_t exp)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type      = EXPR_TYPE_INC;
    expr->line      = line;
    expr->column    = column;
    expr->u.unary   = exp;

    return expr;
}

expr_t expr_new_dec(long line, long column, expr_t exp)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type      = EXPR_TYPE_DEC;
    expr->line      = line;
    expr->column    = column;
    expr->u.unary   = exp;

    return expr;
}

expr_t expr_new_binary(expr_type_t type, long line, long column, expr_t left, expr_t right)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    assert(left != NULL);
    assert(right != NULL);

    expr->type          = type;
    expr->line          = line;
    expr->column        = column;
    expr->u.binary.left = left;
    expr->u.binary.right = right;

    return expr;
}

expr_t expr_new_array_index(long line, long column, expr_t array, expr_t index)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type      = EXPR_TYPE_ARRAY_INDEX;
    expr->line      = line;
    expr->column    = column;
    expr->u.array_index.array = array;
    expr->u.array_index.index = index;

    return expr;
}

expr_t expr_new_array_generate(long line, long column)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }

    expr->type      = EXPR_TYPE_ARRAY_GENERATE;
    expr->line      = line;
    expr->column    = column;

    list_init(expr->u.array_generate.exprs);

    return expr;
}

expr_t expr_new_closure(long line, long column, closure_t closure)
{
    expr_t expr = mem_alloc(sizeof(struct expr_s));
    if (!expr) {
        return NULL;
    }
 
    assert(closure != NULL);

    expr->type          = EXPR_TYPE_CLOSURE;
    expr->line          = line;
    expr->column        = column;
    expr->u.closure     = closure;

    return expr;
}

void expr_free(expr_t expr)
{
    list_iter_t iter = NULL, next_iter = NULL;

    switch (expr->type) {
    case EXPR_TYPE_CHAR:
    case EXPR_TYPE_BOOL:
    case EXPR_TYPE_INT:
    case EXPR_TYPE_LONG:
    case EXPR_TYPE_FLOAT:
    case EXPR_TYPE_DOUBLE:
    case EXPR_TYPE_NULL:
        break;

    case EXPR_TYPE_STRING:
        cstring_free(expr->u.string_value);
        break;

    case EXPR_TYPE_IDENTIFIER:
        cstring_free(expr->u.identifier);
        break;

    case EXPR_TYPE_ADD_ASSIGN:
    case EXPR_TYPE_SUB_ASSIGN:
    case EXPR_TYPE_MUL_ASSIGN:
    case EXPR_TYPE_DIV_ASSIGN:
    case EXPR_TYPE_MOD_ASSIGN:
    case EXPR_TYPE_ASSIGN:
        expr_free(expr->u.assign.lvalue);
        expr_free(expr->u.assign.rvalue);
        break;

    case EXPR_TYPE_CLOSURE:
        closure_free(expr->u.closure);
        break;

    case EXPR_TYPE_CALL:
        cstring_free(expr->u.call.name);
        list_safe_for_each(expr->u.call.args, iter, next_iter) {
            list_erase(*iter);
            expr_free(list_element(iter, expr_t, link));
        }
        break;

    case EXPR_TYPE_PLUS:
    case EXPR_TYPE_MINUS:
    case EXPR_TYPE_INC:
    case EXPR_TYPE_DEC:
    case EXPR_TYPE_NOT:
        expr_free(expr->u.unary);
        break;

    case EXPR_TYPE_BITAND:
    case EXPR_TYPE_BITOR:
    case EXPR_TYPE_XOR:
    case EXPR_TYPE_LEFT_SHIFT:
    case EXPR_TYPE_RIGHT_SHIFT:
    case EXPR_TYPE_LOGIC_RIGHT_SHIFT:
    case EXPR_TYPE_MUL:
    case EXPR_TYPE_DIV:
    case EXPR_TYPE_MOD:
    case EXPR_TYPE_ADD:
    case EXPR_TYPE_SUB:
    case EXPR_TYPE_GT:
    case EXPR_TYPE_GEQ:
    case EXPR_TYPE_LT:
    case EXPR_TYPE_LEQ:
    case EXPR_TYPE_EQ:
    case EXPR_TYPE_NEQ:
    case EXPR_TYPE_AND:
    case EXPR_TYPE_OR:
        expr_free(expr->u.binary.left);
        expr_free(expr->u.binary.right);
        break;
    }

    mem_free(expr);
}