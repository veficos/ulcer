

#include "environment.h"
#include "executor.h"
#include "error.h"
#include "array.h"
#include "eval.h"
#include "heap.h"
#include "expr.h"

#include <math.h>
#include <assert.h>

#define __is_math_operator__(oper)                                            \
    ((oper) == EXPR_TYPE_ADD || (oper) == EXPR_TYPE_SUB ||                    \
     (oper) == EXPR_TYPE_MUL || (oper) == EXPR_TYPE_DIV ||                    \
     (oper) == EXPR_TYPE_MOD)

#define __is_bit_operator__(oper)                                             \
    ((oper) == EXPR_TYPE_BITAND || (oper) == EXPR_TYPE_BITOR ||               \
    (oper) == EXPR_TYPE_XOR || (oper) == EXPR_TYPE_LEFT_SHIFT ||              \
    (oper) == EXPR_TYPE_RIGHT_SHIFT || (oper) == EXPR_TYPE_LOGIC_RIGHT_SHIFT) 

#define __is_compare_operator__(oper)                                         \
    ((oper) == EXPR_TYPE_GT || (oper) == EXPR_TYPE_GEQ ||                     \
     (oper) == EXPR_TYPE_LT || (oper) == EXPR_TYPE_LEQ ||                     \
     (oper) == EXPR_TYPE_EQ || (oper) == EXPR_TYPE_NEQ)

static void __eval_char_expr__(environment_t env, char char_value);
static void __eval_bool_expr__(environment_t env, bool bool_value);
static void __eval_int_expr__(environment_t env, int int_value);
static void __eval_long_expr__(environment_t env, long long_value);
static void __eval_float_expr__(environment_t env, float float_value);
static void __eval_double_expr__(environment_t env, double double_value);
static void __eval_string_expr__(environment_t env, cstring_t string_value);
static void __eval_closure_expr__(environment_t env, closure_t closure);
static void __eval_null_expr__(environment_t env);
static void __eval_call_expr__(environment_t env, expr_t call_expr);
static void __eval_closure_call_expr__(environment_t env, closure_t closure);
static void __eval_identifier_expr__(environment_t env, cstring_t identifier);
static void __eval_assign_expr__(environment_t env, expr_type_t type, expr_t lvalue_expr, expr_t rvalue_expr);
static void __eval_native_function_call_expr__(environment_t env, expr_call_t call, native_function_pt function);
static void __eval_function_call_expr__(environment_t env, expr_t call_expr, function_t func);
static void __eval_binary_expr__(environment_t env, expr_type_t type, expr_t left, expr_t right);
static void __eval_char_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_bool_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_int_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_long_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_float_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_double_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_string_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_null_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result);
static void __eval_logic_binary_expr__(environment_t env, expr_type_t type, expr_t left, expr_t right);
static void __eval_unary_expr__(environment_t env, expr_type_t type, expr_t unary_expr);

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
        __eval_identifier_expr__(env, expr->u.identifier);
        break;

    //case EXPR_TYPE_CLOSURE:
      //  __eval_closure_expr__(env, expr->u.closure);
        //break;

    case EXPR_TYPE_ADD_ASSIGN:
    case EXPR_TYPE_SUB_ASSIGN:
    case EXPR_TYPE_MUL_ASSIGN:
    case EXPR_TYPE_DIV_ASSIGN:
    case EXPR_TYPE_MOD_ASSIGN:
    case EXPR_TYPE_ASSIGN:
        __eval_assign_expr__(env, expr->type, expr->u.assign.lvalue, expr->u.assign.rvalue);
        break;

    case EXPR_TYPE_CALL:
        __eval_call_expr__(env, expr);
        break;

    case EXPR_TYPE_FLIP:
    case EXPR_TYPE_INC:
    case EXPR_TYPE_DEC:
    case EXPR_TYPE_NOT:
    case EXPR_TYPE_PLUS:
    case EXPR_TYPE_MINUS:
        __eval_unary_expr__(env, expr->type, expr);
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
        __eval_binary_expr__(env, expr->type, expr->u.binary.left, expr->u.binary.right);
        break;

    case EXPR_TYPE_AND:
    case EXPR_TYPE_OR:
        __eval_logic_binary_expr__(env, expr->type, expr->u.binary.left, expr->u.binary.right);
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
/*
static void __eval_closure_expr__(environment_t env, closure_t closure)
{
    struct value_s temp;
    value_t lvalue = NULL;
    value_t value  = NULL; 

    if (closure->init_call) {
        __eval_closure_call_expr__(env, closure);
       
        value                  = &temp;
        value->type            = VALUE_TYPE_CLOSURE;
        value->u.closure_value = closure;
    } else {
        value                  = array_push(env->stack);
        value->type            = VALUE_TYPE_CLOSURE;
        value->u.closure_value = closure;
    }

    if (!cstring_is_empty(closure->name) && !list_is_empty(env->local_context_stack)) {
        lvalue = environment_search_local_lvalue_variable(env, closure->name);
        if (lvalue == NULL) {
            environment_new_local_lvalue_variable(env, closure->name, value);
        } else {
            *lvalue = *value;
        }

    } else {
        lvalue = environment_search_global_lvalue_variable(env, closure->name);
        if (lvalue == NULL) {
            environment_new_global_lvalue_variable(env, closure->name, value);
        } else {
            *lvalue = *value;
        }
    }
}
*/

static void __eval_null_expr__(environment_t env)
{
    value_t value = array_push(env->stack);

    value->type = VALUE_TYPE_NULL;
}

static void __eval_call_expr__(environment_t env, expr_t call_expr)
{
    function_t function;
    
    function = environment_search_function(env, call_expr->u.call.name);
    if (!function) {
        runtime_error(call_expr->line,
                      call_expr->column,
                      "undefined function to '%s'",
                      call_expr->u.call.name);
    }

    switch (function->type) {
    case FUNCTION_TYPE_NATIVE:
        __eval_native_function_call_expr__(env, call_expr->u.call, function->u.native.function);
        break;
    case FUNCTION_TYPE_USER:
        __eval_function_call_expr__(env, call_expr, function);
        break;
    }
}

static void __eval_closure_call_expr__(environment_t env, closure_t closure)
{
    list_iter_t iter;
    list_iter_t parameter;
    cstring_t arg_name;
    value_t arg_value;
    unsigned long argc = 0;
    unsigned long index = 0;

    list_for_each(closure->init_args, iter) {
        eval_expression(env, list_element(iter, expr_t, link));
        argc++;
    }

    environment_push_local_context(env);
    parameter = list_begin(closure->parameters);

    for (; index < argc; index++) {
        if (list_end(closure->parameters) == parameter) {
            continue;
        }

        arg_name = list_element(parameter, parameter_t, link)->name;
        arg_value = (value_t) array_index(env->stack, index);
        environment_new_local_variable_by_identifier(env, arg_name, arg_value);

        parameter = list_next(parameter);
    }

    array_pop_n(env->stack, argc);

    switch (executor_statement(env, closure->block)) {
    case EXECUTOR_RESULT_BREAK:
        runtime_error(closure->line,
                      closure->column,
                      "break outside loop");
        break;
    case EXECUTOR_RESULT_CONTINUE:
        runtime_error(closure->line,
                      closure->column,
                      "continue outside loop");
        break;
    default:
        break;
    }

    environment_pop_local_context(env);

    if (array_is_empty(env->stack)) {
        value_t value = array_push(env->stack);
        value->type   = VALUE_TYPE_NULL;
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

static void __eval_function_call_expr__(environment_t env, expr_t call_expr, function_t func)
{
    list_iter_t iter;
    list_iter_t parameter;
    cstring_t arg_name;
    value_t arg_value;
    unsigned long argc = 0;
    unsigned long index = 0;
  
    list_for_each(call_expr->u.call.args, iter) {
        eval_expression(env, list_element(iter, expr_t, link));
        argc++;
    }

    environment_push_local_context(env);
    parameter = list_begin(func->u.user.parameters);

    for (; index < argc; index++) {
        if (list_end(func->u.user.parameters) == parameter) {
            continue;
        }

        arg_name  = list_element(parameter, parameter_t, link)->name;
        arg_value = (value_t) array_index(env->stack, index);
        environment_new_local_variable_by_identifier(env, arg_name, arg_value);

        parameter = list_next(parameter);
    }

    array_pop_n(env->stack, argc);

    switch (executor_statement(env, func->u.user.block)) {
        case EXECUTOR_RESULT_BREAK:
            runtime_error(call_expr->line, 
                          call_expr->column,
                          "break outside loop");
            break;
        case EXECUTOR_RESULT_CONTINUE:
            runtime_error(call_expr->line, 
                          call_expr->column,
                          "continue outside loop");
            break;
        default:
            break;
    }

    environment_pop_local_context(env);

    if (array_is_empty(env->stack)) {
        value_t value = array_push(env->stack);
        value->type = VALUE_TYPE_NULL;
    }
}

static void __eval_identifier_expr__(environment_t env, cstring_t identifier)
{
    value_t value;
    
    if (!list_is_empty(env->local_context_stack)) {
        value = environment_search_local_lvalue_variable_by_lvalue_expr(env, identifier);
        if (value) {
            *(value_t) array_push(env->stack) = *value;
        } else {
            value = (value_t) array_push(env->stack);
            value->type = VALUE_TYPE_NULL;
        }

    } else {
        value = environment_search_global_lvalue_variable(env, identifier);
        if (value) {
            *(value_t) array_push(env->stack) = *value;
        } else {
            value = (value_t) array_push(env->stack);
            value->type = VALUE_TYPE_NULL;
        }
    }
}

static void __eval_assign_expr__(environment_t env, expr_type_t type, expr_t lvalue_expr, expr_t rvalue_expr)
{
    struct value_s result;
    value_t lvalue;
    value_t expr_value;

    eval_expression(env, rvalue_expr);

    expr_value = (value_t) array_index(env->stack, array_length(env->stack) - 1);
    if (!list_is_empty(env->local_context_stack)) {
        lvalue = environment_search_local_lvalue_variable_by_lvalue_expr(env, lvalue_expr);
        if (lvalue == NULL) {
            environment_new_local_variable_by_lvalue_expr(env, lvalue_expr, expr_value);
            if (expr_value->type == VALUE_TYPE_STRING && (type != EXPR_TYPE_ADD_ASSIGN && type != EXPR_TYPE_ASSIGN)) {
                runtime_error(rvalue_expr->line,
                              rvalue_expr->column,
                              "unsupported operand for : type(null) %s type(%s)",
                              expr_type_string(type),
                              value_type_string(expr_value));
            }
            return;
        } 

    } else {
        lvalue = environment_search_global_lvalue_variable(env, lvalue_expr);
        if (lvalue == NULL) {
            environment_new_global_lvalue_variable_by_value_expr(env, lvalue_expr, expr_value);
            if (expr_value->type == VALUE_TYPE_STRING && (type != EXPR_TYPE_ADD_ASSIGN && type != EXPR_TYPE_ASSIGN)) {
                runtime_error(rvalue_expr->line,
                              rvalue_expr->column,
                              "unsupported operand for : type(null) %s type(%s)",
                              expr_type_string(type),
                              value_type_string(expr_value));
            }
            return;
        } 
    }

    switch (type) {
    case EXPR_TYPE_ADD_ASSIGN:
        type = EXPR_TYPE_ADD;
        break;

    case EXPR_TYPE_SUB_ASSIGN:
        type = EXPR_TYPE_SUB;
        break;

    case EXPR_TYPE_MUL_ASSIGN:
        type = EXPR_TYPE_MUL;
        break;

    case EXPR_TYPE_DIV_ASSIGN:
        type = EXPR_TYPE_DIV;
        break;

    case EXPR_TYPE_MOD_ASSIGN:
        type = EXPR_TYPE_MOD;
        break;

    default:
        *lvalue = *expr_value;
        return;
    }

    if (lvalue->type == VALUE_TYPE_CHAR && expr_value->type == VALUE_TYPE_CHAR ||
        lvalue->type == VALUE_TYPE_INT && expr_value->type == VALUE_TYPE_CHAR ||
        lvalue->type == VALUE_TYPE_CHAR && expr_value->type == VALUE_TYPE_INT ||
        lvalue->type == VALUE_TYPE_LONG && expr_value->type == VALUE_TYPE_CHAR ||
        lvalue->type == VALUE_TYPE_CHAR && expr_value->type == VALUE_TYPE_LONG) {
        __eval_char_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    }else if (lvalue->type == VALUE_TYPE_INT && expr_value->type == VALUE_TYPE_INT) {
        __eval_int_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && expr_value->type == VALUE_TYPE_INT) {
        expr_value->u.float_value = (float)expr_value->u.int_value;
        __eval_float_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_INT && expr_value->type == VALUE_TYPE_FLOAT) {
        lvalue->u.float_value = (float)lvalue->u.int_value;
        __eval_float_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && expr_value->type == VALUE_TYPE_INT) {
        rvalue_expr->u.double_value = (double)rvalue_expr->u.int_value;
        __eval_double_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_INT && expr_value->type == VALUE_TYPE_DOUBLE) {
        lvalue->u.double_value = (double)lvalue->u.int_value;
        __eval_double_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_LONG && expr_value->type == VALUE_TYPE_LONG) {
        __eval_long_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && expr_value->type == VALUE_TYPE_LONG) {
        rvalue_expr->u.float_value = (float)rvalue_expr->u.long_value;
        __eval_float_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_LONG && expr_value->type == VALUE_TYPE_FLOAT) {
        lvalue->u.float_value = (float)lvalue->u.long_value;
        __eval_float_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && expr_value->type == VALUE_TYPE_LONG) {
        rvalue_expr->u.double_value = (double)rvalue_expr->u.long_value;
        __eval_double_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_LONG && expr_value->type == VALUE_TYPE_DOUBLE) {
            lvalue->u.double_value = (double)lvalue->u.long_value;
        __eval_double_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && expr_value->type == VALUE_TYPE_FLOAT) {
        __eval_float_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && expr_value->type == VALUE_TYPE_FLOAT) {
        rvalue_expr->u.double_value = (double)rvalue_expr->u.float_value;
        __eval_double_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_FLOAT && expr_value->type == VALUE_TYPE_DOUBLE) {
            lvalue->u.double_value = (double)lvalue->u.float_value;
        __eval_double_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_DOUBLE && expr_value->type == VALUE_TYPE_DOUBLE) {
        __eval_double_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else if (lvalue->type == VALUE_TYPE_STRING && expr_value->type == VALUE_TYPE_STRING) {
        __eval_string_binary_expr__(env, rvalue_expr->line, rvalue_expr->column, type, lvalue, expr_value, &result);

    } else {
        runtime_error(rvalue_expr->line,
                      rvalue_expr->column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      value_type_string(lvalue),
                      expr_type_string(type),
                      value_type_string(expr_value));
    }

    *lvalue = result;
    *(value_t)array_index(env->stack, array_length(env->stack) - 1) = *lvalue;
    heap_takeover_value(env, lvalue);
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

    } else if (lvalue->type == VALUE_TYPE_NULL || rvalue->type == VALUE_TYPE_NULL) {
        __eval_null_binary_expr__(env, line, column, type, lvalue, rvalue, &result);

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      value_type_string(lvalue),
                      expr_type_string(type),
                      value_type_string(rvalue));
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

    } else if (__is_bit_operator__(type)) {
        result->type = VALUE_TYPE_CHAR;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
    }

    switch (type) {
    case EXPR_TYPE_BITAND:
        result->u.char_value = left->u.char_value & right->u.char_value;
        break;

    case EXPR_TYPE_BITOR:
        result->u.char_value = left->u.char_value | right->u.char_value;
        break;

    case EXPR_TYPE_XOR:
        result->u.char_value = left->u.char_value ^ right->u.char_value;
        break;

    case EXPR_TYPE_LEFT_SHIFT:
        result->u.char_value = left->u.char_value << right->u.char_value;
        break;

    case EXPR_TYPE_RIGHT_SHIFT:
        result->u.char_value = left->u.char_value >> right->u.char_value;
        break;

    case EXPR_TYPE_LOGIC_RIGHT_SHIFT:
        result->u.char_value = (char)((unsigned char) left->u.char_value >> (unsigned char) right->u.char_value);
        break;

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
        if (right->u.char_value == 0) {
            runtime_error(line, column, "div by zero");
        }
        result->u.char_value = left->u.char_value / right->u.char_value;
        break;
    case EXPR_TYPE_MOD:
        if (right->u.char_value == 0) {
            runtime_error(line, column, "div by zero");
        }
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
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
    }
}

static void __eval_int_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_INT;

    } else if (__is_bit_operator__(type)) {
        result->type = VALUE_TYPE_INT;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
    }
    
    switch (type) {
    case EXPR_TYPE_BITAND:
        result->u.int_value = left->u.int_value & right->u.int_value;
        break;

    case EXPR_TYPE_BITOR:
        result->u.int_value = left->u.int_value | right->u.int_value;
        break;

    case EXPR_TYPE_XOR:
        result->u.int_value = left->u.int_value ^ right->u.int_value;
        break;

    case EXPR_TYPE_LEFT_SHIFT:
        result->u.int_value = left->u.int_value << right->u.int_value;
        break;

    case EXPR_TYPE_RIGHT_SHIFT:
        result->u.int_value = left->u.int_value >> right->u.int_value;
        break;

    case EXPR_TYPE_LOGIC_RIGHT_SHIFT:
        result->u.int_value = (int)((unsigned int)left->u.int_value >> (unsigned int)right->u.int_value);
        break;

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
        if (right->u.char_value == 0) {
            runtime_error(line, column, "div by zero");
        }
        result->u.int_value = left->u.int_value / right->u.int_value;
        break;
    case EXPR_TYPE_MOD:
        if (right->u.char_value == 0) {
            runtime_error(line, column, "div by zero");
        }
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

    } else if (__is_bit_operator__(type)) {
        result->type = VALUE_TYPE_LONG;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
    }

    switch (type) {
    case EXPR_TYPE_BITAND:
        result->u.long_value = left->u.long_value & right->u.long_value;
        break;

    case EXPR_TYPE_BITOR:
        result->u.long_value = left->u.long_value | right->u.long_value;
        break;

    case EXPR_TYPE_XOR:
        result->u.long_value = left->u.long_value ^ right->u.long_value;
        break;

    case EXPR_TYPE_LEFT_SHIFT:
        result->u.long_value = left->u.long_value << right->u.long_value;
        break;

    case EXPR_TYPE_RIGHT_SHIFT:
        result->u.long_value = left->u.long_value >> right->u.long_value;
        break;

    case EXPR_TYPE_LOGIC_RIGHT_SHIFT:
        result->u.long_value = (long)((unsigned long)left->u.long_value >> (unsigned long)right->u.long_value);
        break;

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
        if (right->u.char_value == 0) {
            runtime_error(line, column, "div by zero");
        }
        result->u.long_value = left->u.long_value / right->u.long_value;
        break;
    case EXPR_TYPE_MOD:
        if (right->u.char_value == 0) {
            runtime_error(line, column, "div by zero");
        }
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
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
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
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
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
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
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

static void __eval_null_binary_expr__(environment_t env, long line, long column, expr_type_t type, value_t left, value_t right, value_t result)
{
    if (type == EXPR_TYPE_EQ || type == EXPR_TYPE_NEQ) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error(line,
                      column,
                      "unsupported operand for : type(%s) %s type(%s)",
                      value_type_string(left),
                      expr_type_string(type),
                      value_type_string(right));
    }

    switch (type) {
    case EXPR_TYPE_EQ:
        result->u.bool_value = left->type == VALUE_TYPE_NULL && left->type == VALUE_TYPE_NULL;
        break;
    case EXPR_TYPE_NEQ:
        result->u.bool_value = !(left->type == VALUE_TYPE_NULL && left->type == VALUE_TYPE_NULL);
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
                      value_type_string(lvalue),
                      expr_type_string(type));
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
                      value_type_string(lvalue),
                      expr_type_string(type),
                      value_type_string(rvalue));
    }

    eval_result = rvalue->u.bool_value;

eval_success:
    result = (value_t) array_push(env->stack);
    result->type         = VALUE_TYPE_BOOL;
    result->u.bool_value = eval_result;
}

static void __eval_unary_expr__(environment_t env, expr_type_t type, expr_t unary_expr)
{
    value_t value;

    eval_expression(env, unary_expr->u.unary);

    value = (value_t) array_index(env->stack, array_length(env->stack) - 1);

    switch (value->type) {
    case VALUE_TYPE_CHAR:
        if (type == EXPR_TYPE_MINUS) {
            value->u.char_value = -value->u.char_value;
        } else if (type == EXPR_TYPE_FLIP) {
            value->u.char_value = ~value->u.char_value;
        }
        break;
    case VALUE_TYPE_BOOL:
        if (type == EXPR_TYPE_NOT) {
            value->u.bool_value = !value->u.bool_value;
        }
        break;
    case VALUE_TYPE_INT:
        if (type == EXPR_TYPE_MINUS) {
            value->u.int_value = -value->u.int_value;
        } else if (type == EXPR_TYPE_INC) {
            value->u.int_value = ++value->u.int_value;
        } else if (type == EXPR_TYPE_DEC) {
            value->u.int_value = --value->u.int_value;
        } else if (type == EXPR_TYPE_FLIP) {
            value->u.int_value = ~value->u.int_value;
        }
        break;
    case VALUE_TYPE_LONG:
        if (type == EXPR_TYPE_MINUS) {
            value->u.long_value = -value->u.long_value;
        } else if (type == EXPR_TYPE_INC) {
            value->u.long_value = ++value->u.long_value;
        } else if (type == EXPR_TYPE_DEC) {
            value->u.long_value = --value->u.long_value;
        } else if (type == EXPR_TYPE_FLIP) {
            value->u.long_value = ~value->u.long_value;
        }
        break;
    case VALUE_TYPE_FLOAT:
        if (type == EXPR_TYPE_MINUS) {
            value->u.float_value = -value->u.float_value;
        }
        break;
    case VALUE_TYPE_DOUBLE:
        if (type == EXPR_TYPE_MINUS) {
            value->u.double_value = -value->u.double_value;
        }
        break;
    default:
        runtime_error(unary_expr->line,
                      unary_expr->column,
                      "unsupported operand for : %s type(%s)",
                      expr_type_string(type),
                      value_type_string(value));
        break;
    }
}

