

#include "eval.h"

static void __eval_char_expr__(char char_value)
{
    value_t value;

    value.type         = VALUE_TYPE_CHAR;
    value.u.char_value = char_value;
}

static void __eval_bool_expr__(bool bool_value)
{
    value_t value;

    value.type         = VALUE_TYPE_BOOL;
    value.u.bool_value = bool_value;


}

static void __eval_int_expr__(int int_value)
{
    value_t value;

    value.type         = VALUE_TYPE_INT;
    value.u.int_value  = int_value;
}

static void __eval_long_expr__(long long_value)
{
    value_t value;

    value.type         = VALUE_TYPE_LONG;
    value.u.long_value = long_value;
}

static void __eval_float_expr__(float float_value)
{
    value_t value;

    value.type          = VALUE_TYPE_FLOAT;
    value.u.float_value = float_value;

}

static void __eval_double_expr__(double double_value)
{
    value_t value;

    value.type           = VALUE_TYPE_DOUBLE;
    value.u.double_value = double_value;
}

static void __eval_string_expr__(cstring_t string_value)
{
    value_t value;

    value.type           = VALUE_TYPE_STRING;
    value.u.string_value = cstring_dup(string_value);

}

static void __eval_null_expr__(void)
{
    value_t value;

    value.type = VALUE_TYPE_NULL;
}

void eval_expression(executor_t inter, expr_t expr)
{
    switch (expr->type) {
    case EXPR_TYPE_CHAR:
        __eval_char_expr__(expr->u.char_value);

    case EXPR_TYPE_BOOL:
        __eval_bool_expr__(expr->u.bool_value);

    case EXPR_TYPE_INT:
        __eval_int_expr__(expr->u.int_value);

    case EXPR_TYPE_LONG:
        __eval_long_expr__(expr->u.long_value);

    case EXPR_TYPE_FLOAT:
        __eval_float_expr__(expr->u.float_value);

    case EXPR_TYPE_DOUBLE:
        __eval_double_expr__(expr->u.double_value);

    case EXPR_TYPE_STRING:
        __eval_string_expr__(expr->u.string_value);

    case EXPR_TYPE_NULL:
        __eval_null_expr__();

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