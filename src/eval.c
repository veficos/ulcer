
#include "environment.h"
#include "array.h"
#include "eval.h"
#include "heap.h"

static void __eval_char_expr__(environment_t env, char char_value);
static void __eval_bool_expr__(environment_t env, bool bool_value);
static void __eval_int_expr__(environment_t env, int int_value);
static void __eval_long_expr__(environment_t env, long long_value);
static void __eval_float_expr__(environment_t env, float float_value);
static void __eval_double_expr__(environment_t env, double double_value);
static void __eval_string_expr__(environment_t env, cstring_t string_value);
static void __eval_null_expr__(environment_t env);
static void __eval_call_expr__(environment_t env, expr_call_t call);

void eval_expression(environment_t env, expr_t expr)
{
    switch (expr->type) {
    case EXPR_TYPE_CHAR:
        __eval_char_expr__(env, expr->u.char_value);
        break;

    case EXPR_TYPE_BOOL:
        __eval_bool_expr__(env, expr->u.bool_value);
        break;

    case EXPR_TYPE_INT:
        __eval_int_expr__(env, expr->u.int_value);
        break;

    case EXPR_TYPE_LONG:
        __eval_long_expr__(env, expr->u.long_value);
        break;

    case EXPR_TYPE_FLOAT:
        __eval_float_expr__(env, expr->u.float_value);
        break;

    case EXPR_TYPE_DOUBLE:
        __eval_double_expr__(env, expr->u.double_value);
        break;

    case EXPR_TYPE_STRING:
        __eval_string_expr__(env, expr->u.string_value);
        break;

    case EXPR_TYPE_NULL:
        __eval_null_expr__(env);
        break;

    case EXPR_TYPE_IDENTIFIER:
        break;

    case EXPR_TYPE_ASSIGN:
    case EXPR_TYPE_CALL:
        __eval_call_expr__(env, expr->u.call);
        break;

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

static void __eval_char_expr__(environment_t env, char char_value)
{
    value_t value = array_push(env->stack);

    value->type         = VALUE_TYPE_CHAR;
    value->u.char_value = char_value;
}

static void __eval_bool_expr__(environment_t env, bool bool_value)
{
    value_t value = array_push(env->stack);

    value->type         = VALUE_TYPE_BOOL;
    value->u.bool_value = bool_value;
}

static void __eval_int_expr__(environment_t env, int int_value)
{
    value_t value = array_push(env->stack);

    value->type         = VALUE_TYPE_INT;
    value->u.int_value  = int_value;
}

static void __eval_long_expr__(environment_t env, long long_value)
{
    value_t value = array_push(env->stack);

    value->type         = VALUE_TYPE_LONG;
    value->u.long_value = long_value;
}

static void __eval_float_expr__(environment_t env, float float_value)
{
    value_t value = array_push(env->stack);

    value->type          = VALUE_TYPE_FLOAT;
    value->u.float_value = float_value;
}

static void __eval_double_expr__(environment_t env, double double_value)
{
    value_t value = array_push(env->stack);

    value->type           = VALUE_TYPE_DOUBLE;
    value->u.double_value = double_value;
}

static void __eval_string_expr__(environment_t env, cstring_t string_value)
{
    value_t value = array_push(env->stack);

    value->u.object_value = heap_alloc_string(env, string_value);
    value->type           = VALUE_TYPE_STRING;
}

static void __eval_null_expr__(environment_t env)
{
    value_t value = array_push(env->stack);

    value->type = VALUE_TYPE_NULL;
}

static void __eval_call_expr__(environment_t env, expr_call_t call)
{

}