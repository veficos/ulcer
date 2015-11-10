
#include "environment.h"
#include "error.h"
#include "array.h"
#include "eval.h"
#include "heap.h"

#include <math.h>

#define __is_math_operator__(oper)                                            \
    ((oper) == EXPR_TYPE_ADD || (oper) == EXPR_TYPE_SUB ||                    \
     (oper) == EXPR_TYPE_MUL || (oper) == EXPR_TYPE_DIV ||                    \
     (oper) == EXPR_TYPE_MOD)

#define __is_compare_operator__(oper)                                         \
    ((oper) == EXPR_TYPE_GT || (oper) == EXPR_TYPE_GEQ ||                     \
     (oper) == EXPR_TYPE_LT || (oper) == EXPR_TYPE_LEQ ||                     \
     (oper) == EXPR_TYPE_EQ || (oper) == EXPR_TYPE_NEQ)

static const char* __get_value_type_string__(value_t value);
static const char* __get_expt_type_string__(expr_type_t expt_type);
static void __eval_char_expr__(environment_t env, char char_value);
static void __eval_bool_expr__(environment_t env, bool bool_value);
static void __eval_int_expr__(environment_t env, int int_value);
static void __eval_long_expr__(environment_t env, long long_value);
static void __eval_float_expr__(environment_t env, float float_value);
static void __eval_double_expr__(environment_t env, double double_value);
static void __eval_string_expr__(environment_t env, cstring_t string_value);
static void __eval_null_expr__(environment_t env);
static void __eval_call_expr__(environment_t env, expr_t call_expr);
static void __eval_native_function_call_expr__(environment_t env, expr_call_t call, native_function_pt function);
static void __eval_binary_expr__(environment_t env, expr_type_t type, expr_t left, expr_t right);
static void __eval_char_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_bool_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_int_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_long_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_float_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_double_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_string_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_logic_binary_expr__(environment_t env, expr_type_t type, expr_t left, expr_t right);

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
        break;

    case EXPR_TYPE_CALL:
        __eval_call_expr__(env, expr);
        break;

    case EXPR_TYPE_PLUS:
    case EXPR_TYPE_MINUS:
        break;

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
        __eval_binary_expr__(env, expr->type, expr->u.binary.left, expr->u.binary.right);
        break;

    case EXPR_TYPE_AND:
    case EXPR_TYPE_OR:
        __eval_logic_binary_expr__(env, expr->type, expr->u.binary.left, expr->u.binary.right);
        break;
    }
}

static const char* __get_value_type_string__(value_t value)
{
    switch (value->type) {
    case VALUE_TYPE_CHAR:
        return "char";
    case VALUE_TYPE_BOOL:
        return "bool";
    case VALUE_TYPE_INT:
        return "int";
    case VALUE_TYPE_LONG:
        return "long";
    case VALUE_TYPE_FLOAT:
        return "float";
    case VALUE_TYPE_DOUBLE:
        return "double";
    case VALUE_TYPE_STRING:
        return "string";
    case VALUE_TYPE_NULL:
        return "null";
    }

    return "unkown";
}

static const char* __get_expt_type_string__(expr_type_t expr_type)
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
    case EXPR_TYPE_CALL:
        return "function call";
    case EXPR_TYPE_PLUS:
        return "+";
    case EXPR_TYPE_MINUS:
        return "-";
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

    return "unkown";
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

static void __eval_call_expr__(environment_t env, expr_t call_expr)
{
    function_t function;
    
    function = environment_search_function(env, call_expr->u.call.function_name);
    if (!function) {
        runtime_error(call_expr->line,
                      call_expr->column,
                      "undefined reference to '%s'",
                      call_expr->u.call.function_name);
    }

    switch (function->type) {
    case FUNCTION_TYPE_NATIVE:
        __eval_native_function_call_expr__(env, call_expr->u.call, function->u.native.function);
        break;
    }
}

static void __eval_native_function_call_expr__(environment_t env, expr_call_t call, native_function_pt function)
{
    list_iter_t iter;
    unsigned long argc = 0;

    list_for_each(call.args, iter) {
        eval_expression(env, list_element(iter, expr_t, link));
        argc++;
    }

    function(env->stack);

    array_pop_n(env->stack, argc);
}

static void __eval_binary_expr__(environment_t env, expr_type_t type, expr_t left, expr_t right)
{
    struct value_s l, r, result;
    value_t lvalue = &l;
    value_t rvalue = &r;
    value_t p;
    long line, column;

    eval_expression(env, left);
    eval_expression(env, right);

    *rvalue = *(value_t) array_index(env->stack, array_length(env->stack) - 1);
    *lvalue = *(value_t) array_index(env->stack, array_length(env->stack) - 2);

    line   = left->line;
    column = left->column;

    if (lvalue->type == VALUE_TYPE_CHAR && rvalue->type == VALUE_TYPE_CHAR ||
        lvalue->type == VALUE_TYPE_INT && rvalue->type == VALUE_TYPE_CHAR ||
        lvalue->type == VALUE_TYPE_CHAR && rvalue->type == VALUE_TYPE_INT ||
        lvalue->type == VALUE_TYPE_LONG && rvalue->type == VALUE_TYPE_CHAR ||
        lvalue->type == VALUE_TYPE_CHAR && rvalue->type == VALUE_TYPE_LONG) {
        __eval_char_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_BOOL && rvalue->type == VALUE_TYPE_BOOL) {
        __eval_bool_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_INT && rvalue->type == VALUE_TYPE_INT) {
        __eval_int_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && rvalue->type == VALUE_TYPE_INT) {
        rvalue->u.float_value = (float)rvalue->u.int_value;
        __eval_float_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_INT && rvalue->type == VALUE_TYPE_FLOAT) {
        lvalue->u.float_value = (float)lvalue->u.int_value;
        __eval_float_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && rvalue->type == VALUE_TYPE_INT) {
        rvalue->u.double_value = (double)rvalue->u.int_value;
        __eval_double_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_INT && rvalue->type == VALUE_TYPE_DOUBLE) {
        lvalue->u.double_value = (double)lvalue->u.int_value;
        __eval_double_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_LONG && rvalue->type == VALUE_TYPE_LONG) {
        __eval_long_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && rvalue->type == VALUE_TYPE_LONG) {
        rvalue->u.float_value = (float)rvalue->u.long_value;
        __eval_float_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_LONG && rvalue->type == VALUE_TYPE_FLOAT) {
        lvalue->u.float_value = (float)lvalue->u.long_value;
        __eval_float_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && rvalue->type == VALUE_TYPE_LONG) {
        rvalue->u.double_value = (double)rvalue->u.long_value;
        __eval_double_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_LONG && rvalue->type == VALUE_TYPE_DOUBLE) {
        lvalue->u.double_value = (double)lvalue->u.long_value;
        __eval_double_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && rvalue->type == VALUE_TYPE_FLOAT) {
        __eval_float_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && rvalue->type == VALUE_TYPE_FLOAT) {
        rvalue->u.double_value = (double)rvalue->u.float_value;
        __eval_double_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && rvalue->type == VALUE_TYPE_DOUBLE) {
        lvalue->u.double_value = (double)lvalue->u.float_value;
        __eval_double_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && rvalue->type == VALUE_TYPE_DOUBLE) {
        __eval_double_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_STRING && rvalue->type == VALUE_TYPE_STRING) {
        __eval_string_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else if (lvalue->type == VALUE_TYPE_NULL && rvalue->type == VALUE_TYPE_NULL) {

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(lvalue),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(rvalue));
    }

    array_pop_n(env->stack, 2);

    p = (value_t) array_push(env->stack);
    *p = result;

    heap_takeover_value(env, p);
}

static void __eval_char_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_CHAR;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(left),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(right));
    }

    switch (type) {
    case EXPR_TYPE_ADD:
        result->u.char_value = left->u.char_value + right->u.char_value;
        break;
    case EXPR_TYPE_SUB:
        result->u.char_value = left->u.char_value - right->u.char_value;
        break;
    case EXPR_TYPE_MUL:
        result->u.char_value = left->u.char_value * right->u.char_value;
        break;
    case EXPR_TYPE_DIV:
        result->u.char_value = left->u.char_value / right->u.char_value;
        break;
    case EXPR_TYPE_MOD:
        result->u.char_value = left->u.char_value % right->u.char_value;
        break;
    case EXPR_TYPE_GT:
        result->u.bool_value = left->u.char_value > right->u.char_value;
        break;
    case EXPR_TYPE_GEQ:
        result->u.bool_value = left->u.char_value >= right->u.char_value;
        break;
    case EXPR_TYPE_LT:
        result->u.bool_value = left->u.char_value < right->u.char_value;
        break;
    case EXPR_TYPE_LEQ:
        result->u.bool_value = left->u.char_value <= right->u.char_value;
        break;
    case EXPR_TYPE_EQ:
        result->u.bool_value = left->u.char_value == right->u.char_value;
        break;
    case EXPR_TYPE_NEQ:
        result->u.bool_value = left->u.char_value != right->u.char_value;
        break;
    }
}

static void __eval_bool_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (type == EXPR_TYPE_EQ) {
        result->u.bool_value = left->u.bool_value == right->u.bool_value;

    } else if (type == EXPR_TYPE_NEQ) {
        result->u.bool_value = left->u.bool_value != right->u.bool_value;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(left),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(right));
    }
}

static void __eval_int_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_INT;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(left),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(right));
    }
    
    switch (type) {
    case EXPR_TYPE_ADD:
        result->u.int_value = left->u.int_value + right->u.int_value;
        break;
    case EXPR_TYPE_SUB:
        result->u.int_value = left->u.int_value - right->u.int_value;
        break;
    case EXPR_TYPE_MUL:
        result->u.int_value = left->u.int_value * right->u.int_value;
        break;
    case EXPR_TYPE_DIV:
        result->u.int_value = left->u.int_value / right->u.int_value;
        break;
    case EXPR_TYPE_MOD:
        result->u.int_value = left->u.int_value % right->u.int_value;
        break;
    case EXPR_TYPE_GT:
        result->u.bool_value = left->u.int_value > right->u.int_value;
        break;
    case EXPR_TYPE_GEQ:
        result->u.bool_value = left->u.int_value >= right->u.int_value;
        break;
    case EXPR_TYPE_LT:
        result->u.bool_value = left->u.int_value < right->u.int_value;
        break;
    case EXPR_TYPE_LEQ:
        result->u.bool_value = left->u.int_value <= right->u.int_value;
        break;
    case EXPR_TYPE_EQ:
        result->u.bool_value = left->u.int_value == right->u.int_value;
        break;
    case EXPR_TYPE_NEQ:
        result->u.bool_value = left->u.int_value != right->u.int_value;
        break;
    }
}

static void __eval_long_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_LONG;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(left),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(right));
    }

    switch (type) {
    case EXPR_TYPE_ADD:
        result->u.long_value = left->u.long_value + right->u.long_value;
        break;
    case EXPR_TYPE_SUB:
        result->u.long_value = left->u.long_value - right->u.long_value;
        break;
    case EXPR_TYPE_MUL:
        result->u.long_value = left->u.long_value * right->u.long_value;
        break;
    case EXPR_TYPE_DIV:
        result->u.long_value = left->u.long_value / right->u.long_value;
        break;
    case EXPR_TYPE_MOD:
        result->u.long_value = left->u.long_value % right->u.long_value;
        break;
    case EXPR_TYPE_GT:
        result->u.bool_value = left->u.long_value > right->u.long_value;
        break;
    case EXPR_TYPE_GEQ:
        result->u.bool_value = left->u.long_value >= right->u.long_value;
        break;
    case EXPR_TYPE_LT:
        result->u.bool_value = left->u.long_value < right->u.long_value;
        break;
    case EXPR_TYPE_LEQ:
        result->u.bool_value = left->u.long_value <= right->u.long_value;
        break;
    case EXPR_TYPE_EQ:
        result->u.bool_value = left->u.long_value == right->u.long_value;
        break;
    case EXPR_TYPE_NEQ:
        result->u.bool_value = left->u.long_value != right->u.long_value;
        break;
    }
}

static void __eval_float_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_FLOAT;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(left),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(right));
    }

    switch (type) {
    case EXPR_TYPE_ADD:
        result->u.float_value = left->u.float_value + right->u.float_value;
        break;
    case EXPR_TYPE_SUB:
        result->u.float_value = left->u.float_value - right->u.float_value;
        break;
    case EXPR_TYPE_MUL:
        result->u.float_value = left->u.float_value * right->u.float_value;
        break;
    case EXPR_TYPE_DIV:
        result->u.float_value = left->u.float_value / right->u.float_value;
        break;
    case EXPR_TYPE_MOD:
        result->u.float_value =(float)fmod((double)left->u.float_value, (double)right->u.float_value);
        break;
    case EXPR_TYPE_GT:
        result->u.bool_value = left->u.float_value > right->u.float_value;
        break;
    case EXPR_TYPE_GEQ:
        result->u.bool_value = left->u.float_value >= right->u.float_value;
        break;
    case EXPR_TYPE_LT:
        result->u.bool_value = left->u.float_value < right->u.float_value;
        break;
    case EXPR_TYPE_LEQ:
        result->u.bool_value = left->u.float_value <= right->u.float_value;
        break;
    case EXPR_TYPE_EQ:
        result->u.bool_value = left->u.float_value == right->u.float_value;
        break;
    case EXPR_TYPE_NEQ:
        result->u.bool_value = left->u.float_value != right->u.float_value;
        break;
    }
}

static void __eval_double_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_DOUBLE;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(left),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(right));
    }

    switch (type) {
    case EXPR_TYPE_ADD:
        result->u.double_value = left->u.double_value + right->u.double_value;
        break;
    case EXPR_TYPE_SUB:
        result->u.double_value = left->u.double_value - right->u.double_value;
        break;
    case EXPR_TYPE_MUL:
        result->u.double_value = left->u.double_value * right->u.double_value;
        break;
    case EXPR_TYPE_DIV:
        result->u.double_value = left->u.double_value / right->u.double_value;
        break;
    case EXPR_TYPE_MOD:
        result->u.double_value = fmod(left->u.double_value, right->u.double_value);
        break;
    case EXPR_TYPE_GT:
        result->u.bool_value = left->u.double_value > right->u.double_value;
        break;
    case EXPR_TYPE_GEQ:
        result->u.bool_value = left->u.double_value >= right->u.double_value;
        break;
    case EXPR_TYPE_LT:
        result->u.bool_value = left->u.double_value < right->u.double_value;
        break;
    case EXPR_TYPE_LEQ:
        result->u.bool_value = left->u.double_value <= right->u.double_value;
        break;
    case EXPR_TYPE_EQ:
        result->u.bool_value = left->u.double_value == right->u.double_value;
        break;
    case EXPR_TYPE_NEQ:
        result->u.bool_value = left->u.double_value != right->u.double_value;
        break;
    }
}

static void __eval_string_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (type == EXPR_TYPE_ADD) {
        result->type = VALUE_TYPE_STRING;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(left),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(right));
    }

    switch (type) {
    case EXPR_TYPE_ADD:
        list_erase(left->u.object_value->link);
        result->u.object_value = left->u.object_value;
        result->u.object_value->u.string = cstring_cat(result->u.object_value->u.string, right->u.object_value->u.string);
        break;
    case EXPR_TYPE_GT:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) > 0;
        break;
    case EXPR_TYPE_GEQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) >= 0;
        break;
    case EXPR_TYPE_LT:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) < 0;
        break;
    case EXPR_TYPE_LEQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) <= 0;
        break;
    case EXPR_TYPE_EQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) == 0;
        break;
    case EXPR_TYPE_NEQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) != 0;
        break;
    }
}

static void __eval_logic_binary_expr__(environment_t env, expr_type_t type, expr_t left, expr_t right)
{
    struct value_s l, r;
    value_t lvalue = &l;
    value_t rvalue = &r;
    value_t result;
    bool eval_result = false;

    eval_expression(env, left);
    *lvalue = *(value_t) array_index(env->stack, array_length(env->stack) - 1);
    array_pop(env->stack);

    if (lvalue->type != VALUE_TYPE_BOOL) {
        runtime_error(left->line,
                      left->column,
                      "unsupported operand for : type(%s) %s",
                      __get_value_type_string__(lvalue),
                      __get_expt_type_string__(type));
    }

    switch (type) {
    case EXPR_TYPE_AND:
        if (lvalue->u.bool_value == false) {
            goto eval_success;
        }
        break;
    case EXPR_TYPE_OR:
        if (lvalue->u.bool_value == true) {
            eval_result = true;
            goto eval_success;
        }
        break;
    }

    eval_expression(env, right);
    *rvalue = *(value_t) array_index(env->stack, array_length(env->stack) - 1);
    array_pop(env->stack);

    if (rvalue->type != VALUE_TYPE_BOOL) {
        runtime_error(left->line,
                      left->column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      __get_value_type_string__(lvalue),
                      __get_expt_type_string__(type),
                      __get_value_type_string__(rvalue));
    }

    eval_result = rvalue->u.bool_value;

eval_success:
    result = (value_t) array_push(env->stack);
    result->type         = VALUE_TYPE_BOOL;
    result->u.bool_value = eval_result;
}