

#include "eval.h"
#include "array.h"

static void __eval_char_expr__(executor_t exec, char char_value)
{
    value_t* value;

    value = (value_t*) array_push(exec->stack);

    value->type         = VALUE_TYPE_CHAR;
    value->u.char_value = char_value;
}

static void __eval_bool_expr__(executor_t exec, bool bool_value)
{
    value_t* value;

    value = (value_t*)array_push(exec->stack);

    value->type         = VALUE_TYPE_BOOL;
    value->u.bool_value = bool_value;
}

static void __eval_int_expr__(executor_t exec, int int_value)
{
    value_t* value;

    value = (value_t*)array_push(exec->stack);

    value->type         = VALUE_TYPE_INT;
    value->u.int_value  = int_value;
}

static void __eval_long_expr__(executor_t exec, long long_value)
{
    value_t* value;

    value = (value_t*)array_push(exec->stack);

    value->type         = VALUE_TYPE_LONG;
    value->u.long_value = long_value;
}

static void __eval_float_expr__(executor_t exec, float float_value)
{
    value_t* value;

    value = (value_t*)array_push(exec->stack);

    value->type          = VALUE_TYPE_FLOAT;
    value->u.float_value = float_value;
}

static void __eval_double_expr__(executor_t exec, double double_value)
{
    value_t* value;

    value = (value_t*)array_push(exec->stack);

    value->type           = VALUE_TYPE_DOUBLE;
    value->u.double_value = double_value;
}

static void __eval_string_expr__(executor_t exec, cstring_t string_value)
{
    value_t* value;

    value = (value_t*)array_push(exec->stack);

    value->type           = VALUE_TYPE_STRING;
}

static void __eval_null_expr__(executor_t exec)
{
    value_t* value;

    value = (value_t*)array_push(exec->stack);

    value->type = VALUE_TYPE_NULL;
}

void eval_expression(executor_t exec, expr_t expr)
{
    switch (expr->type) {
    case EXPR_TYPE_CHAR:
        __eval_char_expr__(exec, expr->u.char_value);

    case EXPR_TYPE_BOOL:
        __eval_bool_expr__(exec, expr->u.bool_value);

    case EXPR_TYPE_INT:
        __eval_int_expr__(exec, expr->u.int_value);

    case EXPR_TYPE_LONG:
        __eval_long_expr__(exec, expr->u.long_value);

    case EXPR_TYPE_FLOAT:
        __eval_float_expr__(exec, expr->u.float_value);

    case EXPR_TYPE_DOUBLE:
        __eval_double_expr__(exec, expr->u.double_value);

    case EXPR_TYPE_STRING:
        __eval_string_expr__(exec, expr->u.string_value);

    case EXPR_TYPE_NULL:
        __eval_null_expr__(exec);

    case EXPR_TYPE_IDENTIFIER:
    case EXPR_TYPE_ASSIGN:
    case EXPR_TYPE_CALL:
    case EXPR_TYPE_PLUS:
    case EXPR_TYPE_MINUS:
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
        break;
    case EXPR_TYPE_OR:
        break;
    }
}