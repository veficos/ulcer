

#include "statement.h"
#include "executor.h"
#include "expression.h"
#include "environment.h"
#include "alloc.h"
#include "error.h"
#include "heap.h"

#include <math.h>
#include <stdio.h>
#include <assert.h>

#define __is_math_operator__(oper)                                            \
    ((oper) == EXPRESSION_TYPE_ADD || (oper) == EXPRESSION_TYPE_SUB ||        \
     (oper) == EXPRESSION_TYPE_MUL || (oper) == EXPRESSION_TYPE_DIV ||        \
     (oper) == EXPRESSION_TYPE_MOD)

#define __is_bit_operator__(oper)                                                \
    ((oper) == EXPRESSION_TYPE_BITAND || (oper) == EXPRESSION_TYPE_BITOR ||      \
     (oper) == EXPRESSION_TYPE_XOR    || (oper) == EXPRESSION_TYPE_LEFT_SHIFT || \
     (oper) == EXPRESSION_TYPE_RIGHT_SHIFT || (oper) == EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT) 

#define __is_compare_operator__(oper)                                         \
    ((oper) == EXPRESSION_TYPE_GT || (oper) == EXPRESSION_TYPE_GEQ ||         \
     (oper) == EXPRESSION_TYPE_LT || (oper) == EXPRESSION_TYPE_LEQ ||         \
     (oper) == EXPRESSION_TYPE_EQ || (oper) == EXPRESSION_TYPE_NEQ)

const char* get_expression_type_string(expression_type_t type);
const char* get_value_type_string(value_type_t type);

static void         __evaluator_identifier_expression__(environment_t env, expression_t lexpr, bool toplevel);
static value_t      __evaluator_search_function__(environment_t env, expression_t function_expr, bool toplevel);
static value_t      __evaluator_search_variable__(environment_t env, expression_t lexpr, bool toplevel);
static value_t      __evaluator_get_lvalue__(environment_t env, expression_t lexpr, bool toplevel);
static void         __evaluator_call_expression__(environment_t env, expression_t call_expr, bool toplevel);
static void         __evaluator_function_call_expression__(environment_t env, value_t function_value, list_t args, bool toplevel);
static void         __evaluator_native_function_call_expression__(environment_t env, value_t function_value, list_t args);
static void         __evaluator_assign_expression__(environment_t env, expression_type_t type, expression_t lvalue_expr, expression_t rvalue_expr, bool toplevel);
static void         __evaluator_do_assign_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_unary_expression__(environment_t env, expression_t expr, bool toplevel);
static void         __evaluator_inc_dec_expression__(environment_t env, expression_t expr, bool toplevel);
static void         __evaluator_binary_expression__(environment_t env, expression_type_t type, expression_t left_expr, expression_t right_expr, bool toplevel);
static value_type_t __evaluator_implicit_cast_expression__(value_t left_value, value_t right_value);
static void         __evaluator_char_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_bool_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_int_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_long_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_float_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_double_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_string_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_null_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
static void         __evaluator_logic_binary_expression__(environment_t env, expression_type_t type, expression_t left_expr, expression_t right_expr, bool toplevel);
static value_t      __evaluator_index_expression__(environment_t env, expression_t expr, bool toplevel);

void evaluator_expression(environment_t env, expression_t expr, bool toplevel)
{
    value_t value = NULL;

    switch (expr->type) {
    case EXPRESSION_TYPE_CHAR:
        environment_push_char(env, expr->u.char_expr);
        break;

    case EXPRESSION_TYPE_BOOL:
        environment_push_bool(env, expr->u.bool_expr);
        break;

    case EXPRESSION_TYPE_INT:
        environment_push_int(env, expr->u.int_expr);
        break;
        
    case EXPRESSION_TYPE_LONG:
        environment_push_long(env, expr->u.long_expr);
        break;

    case EXPRESSION_TYPE_FLOAT:
        environment_push_float(env, expr->u.float_expr);
        break;

    case EXPRESSION_TYPE_DOUBLE:
        environment_push_double(env, expr->u.double_expr);
        break;

    case EXPRESSION_TYPE_STRING:
        environment_push_string(env, expr->u.string_expr);
        break;

    case EXPRESSION_TYPE_NULL:
        environment_push_null(env);
        break;

    case EXPRESSION_TYPE_FUNCTION:
        environment_push_function(env, expr->u.function_expr);
        break;

    case EXPRESSION_TYPE_IDENTIFIER:
        __evaluator_identifier_expression__(env, expr, toplevel);
        break;

    case EXPRESSION_TYPE_ASSIGN:
    case EXPRESSION_TYPE_ADD_ASSIGN:
    case EXPRESSION_TYPE_SUB_ASSIGN:
    case EXPRESSION_TYPE_MUL_ASSIGN:
    case EXPRESSION_TYPE_DIV_ASSIGN:
    case EXPRESSION_TYPE_MOD_ASSIGN:
    case EXPRESSION_TYPE_BITAND_ASSIGN:
    case EXPRESSION_TYPE_BITOR_ASSIGN:
    case EXPRESSION_TYPE_XOR_ASSIGN:
    case EXPRESSION_TYPE_LEFT_SHIFT_ASSIGN:
    case EXPRESSION_TYPE_RIGHT_SHIFT_ASSIGN:
    case EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT_ASSIGN:
        __evaluator_assign_expression__(env, expr->type, expr->u.assign_expr->lvalue_expr, expr->u.assign_expr->rvalue_expr, toplevel);
        break;

    case EXPRESSION_TYPE_CALL:
        __evaluator_call_expression__(env, expr, toplevel);
        break;

    case EXPRESSION_TYPE_CPL:
    case EXPRESSION_TYPE_NOT:
    case EXPRESSION_TYPE_PLUS:
    case EXPRESSION_TYPE_MINUS:
        __evaluator_unary_expression__(env, expr, toplevel);
        break;

    case EXPRESSION_TYPE_INC:
    case EXPRESSION_TYPE_DEC:
        __evaluator_inc_dec_expression__(env, expr, toplevel);
        break;
 
    /* binary expression */
    case EXPRESSION_TYPE_BITAND:
    case EXPRESSION_TYPE_BITOR:
    case EXPRESSION_TYPE_XOR:
    case EXPRESSION_TYPE_LEFT_SHIFT:
    case EXPRESSION_TYPE_RIGHT_SHIFT:
    case EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT:
    case EXPRESSION_TYPE_MUL:
    case EXPRESSION_TYPE_DIV:
    case EXPRESSION_TYPE_MOD:
    case EXPRESSION_TYPE_ADD:
    case EXPRESSION_TYPE_SUB:
    case EXPRESSION_TYPE_GT:
    case EXPRESSION_TYPE_GEQ:
    case EXPRESSION_TYPE_LT:
    case EXPRESSION_TYPE_LEQ:
    case EXPRESSION_TYPE_EQ:
    case EXPRESSION_TYPE_NEQ:
        __evaluator_binary_expression__(env, expr->type, expr->u.binary_expr->left, expr->u.binary_expr->right, toplevel);
        break;

    case EXPRESSION_TYPE_AND:
    case EXPRESSION_TYPE_OR:
        __evaluator_logic_binary_expression__(env, expr->type, expr->u.binary_expr->left, expr->u.binary_expr->right, toplevel);
        break;

    case EXPRESSION_TYPE_ARRAY_GENERATE:
        environment_push_array_generate(env, expr->u.array_generate_expr, toplevel);
        break;

    case EXPRESSION_TYPE_TABLE_GENERATE:
        environment_push_table_generate(env, expr->u.table_generate_expr, toplevel);
        break;

    case EXPRESSION_TYPE_ARRAY_PUSH:
    case EXPRESSION_TYPE_ARRAY_POP:
    case EXPRESSION_TYPE_TABLE_DOT_MEMBER:
    case EXPRESSION_TYPE_INDEX:
        value = __evaluator_get_lvalue__(env, expr, toplevel);
        if (value) {
            list_push_back(env->stack, value_dup(value)->link);
        }
        break;
    default:
       break;
    }
}

void evaluator_binary_value(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    switch (__evaluator_implicit_cast_expression__(left, right)) {
    case VALUE_TYPE_CHAR:
        __evaluator_char_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_BOOL:
        __evaluator_bool_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_INT:
        __evaluator_int_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_LONG:
        __evaluator_long_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_FLOAT:
        __evaluator_float_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_DOUBLE:
        __evaluator_double_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_STRING:
        __evaluator_string_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_NULL:
        __evaluator_null_binary_expression__(env, line, column, type, left, right);
        break;

    case VALUE_TYPE_TABLE:
    case VALUE_TYPE_REFERENCE:
    case VALUE_TYPE_POINTER:
    case VALUE_TYPE_FUNCTION:
    case VALUE_TYPE_NATIVE_FUNCTION:
    case VALUE_TYPE_ARRAY:
    default:
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)",
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
        break;
    }
}

static void __evaluator_identifier_expression__(environment_t env, expression_t lexpr, bool toplevel)
{
    value_t value = __evaluator_search_variable__(env, lexpr, toplevel);

    if (value) {
        list_push_back(env->stack, value_dup(value)->link);
    } else {
        environment_push_null(env);
    }
}

static value_t __evaluator_search_identifier_variable__(environment_t env, cstring_t identifier, bool toplevel) 
{
    list_iter_t iter;

    list_reverse_for_each(env->local_context_stack, iter) {
        local_context_t context = list_element(iter, local_context_t, link);

        value_t value = table_search_member(context->object->u.table, identifier);

        if (value) {
            return value;
        }
    }

    return table_search_member(environment_get_global_table(env), identifier);
}

static value_t __evaluator_search_function__(environment_t env, expression_t function_expr, bool toplevel)
{
    value_t value;

    switch (function_expr->type) {
    case EXPRESSION_TYPE_IDENTIFIER:
        value = __evaluator_search_identifier_variable__(env, function_expr->u.identifier_expr, toplevel);
        if (value) {
            value = value_dup(value);
        } else {
            value = value_new(VALUE_TYPE_NULL);
        }
        break;

    default:
        evaluator_expression(env, function_expr, toplevel);
        value = list_element(list_rbegin(env->stack), value_t, link);
        list_pop_back(env->stack);
        break;
    }

    return value;
}

static value_t __evaluator_search_variable__(environment_t env, expression_t lexpr, bool toplevel)
{
    switch (lexpr->type) {
    case EXPRESSION_TYPE_IDENTIFIER:
        return __evaluator_search_identifier_variable__(env, lexpr->u.identifier_expr, toplevel);
    default:
        return NULL;
    }
    return NULL;
}

static value_t __evaluator_get_variable_lvalue__(environment_t env, cstring_t identifier, bool toplevel)
{
    value_t value = NULL;

    value = __evaluator_search_identifier_variable__(env, identifier, toplevel);

   // if (!value && toplevel) {
        if (list_is_empty(env->local_context_stack)) {
            value = table_new_member(environment_get_global_table(env), identifier);
        } else {
            local_context_t context = list_element(list_rbegin(env->local_context_stack), local_context_t, link);
            value = table_new_member(context->object->u.table, identifier);
        }
  //  }

    assert(value != NULL);
    return value;
}

static value_t __evaluator_index_expression__(environment_t env, expression_t expr, bool toplevel)
{
    value_t value;
    value_t index_value;

    evaluator_expression(env, expr->u.index_expr->dict, toplevel);

    value = list_element(list_rbegin(env->stack), value_t, link);

    list_pop_back(env->stack);

    evaluator_expression(env, expr->u.index_expr->index, toplevel);

    index_value = list_element(list_rbegin(env->stack), value_t, link);

    list_pop_back(env->stack);

    switch (value->type) {
    case VALUE_TYPE_ARRAY:
        value = *(value_t*)array_index(value->u.object_value->u.array, index_value->u.int_value);
        break;
    }

    return value;
}

static value_t __evaluator_get_lvalue__(environment_t env, expression_t expr, bool toplevel)
{
    value_t value;
 
    switch (expr->type) {
    case EXPRESSION_TYPE_IDENTIFIER:
        value = __evaluator_get_variable_lvalue__(env, expr->u.identifier_expr, toplevel);
        break;
    case EXPRESSION_TYPE_INDEX:
        value = __evaluator_index_expression__(env, expr, toplevel);
        break;
    default:
        value = NULL;
    }

    return value;
}

static void __evaluator_call_expression__(environment_t env, expression_t call_expr, bool toplevel)
{
    value_t function_value;

    function_value = __evaluator_search_function__(env, call_expr->u.call_expr->function_expr, toplevel);

    if (function_value) {
        environment_push_value_to_function_stack(env, function_value);

        switch (function_value->type) {
        case VALUE_TYPE_FUNCTION:
            __evaluator_function_call_expression__(env, function_value, call_expr->u.call_expr->args, false);
            break;

        case VALUE_TYPE_NATIVE_FUNCTION:
            __evaluator_native_function_call_expression__(env, function_value, call_expr->u.call_expr->args);
            break;

        default:
            runtime_error("(%d, %d): called object type '%s' is not a function", 
                          call_expr->line, 
                          call_expr->column, 
                          get_value_type_string(function_value->type));
            break;
        }

        environment_pop_value_from_function_stack(env);

        value_free(function_value);
    }
}

static void __evaluator_function_call_expression__(environment_t env, value_t function_value, list_t args, bool toplevel)
{
    list_iter_t iter;
    statement_t stmt;
    object_t object;
    executor_result_t result;
    expression_function_t function;
    int scopecount = 0;

    environment_push_local_context(env);

    function = function_value->u.object_value->u.function->f.function_expr;

    list_for_each(function_value->u.object_value->u.function->scopes, iter) {
        object = list_element(iter, object_t, link_scope);
        environment_push_scope_local_context(env, object);
        scopecount++;
    }

    list_for_each(function->block, iter) {
        stmt = list_element(iter, statement_t, link);
        result = executor_statement(env, stmt, false);
        if (result == EXECUTOR_RESULT_RETURN) {
            break;
        } else if (result == EXECUTOR_RESULT_BREAK) {
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "break outside loop");
            break;
        } else if (result == EXECUTOR_RESULT_CONTINUE) {
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "break outside loop");
            break;
        }
    }

    if (result != EXECUTOR_RESULT_RETURN) {
        environment_push_null(env);
    }

    while (scopecount--) {
        environment_pop_local_context(env);
    }

    environment_pop_local_context(env);
}

static void __evaluator_native_function_call_expression__(environment_t env, value_t function_value, list_t args)
{
    list_iter_t iter;
    expression_t expr;
    unsigned int argc;
    native_function_pt native_function;

    native_function = function_value->u.object_value->u.function->f.native_function;

    argc = 0;
    list_for_each(args, iter) {
        expr = list_element(iter, expression_t, link);
        evaluator_expression(env, expr, false);
        argc++;
    }

    native_function(env, argc);
}

static void __evaluator_assign_expression__(environment_t env, expression_type_t type, expression_t lvalue_expr, expression_t rvalue_expr, bool toplevel)
{
    value_t rvalue;
    value_t lvalue;

    evaluator_expression(env, rvalue_expr, toplevel);

    rvalue = list_element(list_rbegin(env->stack), value_t, link);

    lvalue = __evaluator_get_lvalue__(env, lvalue_expr, toplevel);

    __evaluator_do_assign_expression__(env, lvalue_expr->line, lvalue_expr->column, type, lvalue, rvalue);
}

static void __evaluator_do_assign_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    *left = *right;
}

static void __evaluator_unary_expression__(environment_t env, expression_t expr, bool toplevel)
{
    value_t operand;

    assert(expr->type == EXPRESSION_TYPE_PLUS || expr->type == EXPRESSION_TYPE_MINUS ||
           expr->type == EXPRESSION_TYPE_CPL || expr->type == EXPRESSION_TYPE_NOT);

    evaluator_expression(env, expr->u.unary_expr, toplevel);

    operand = list_element(list_rbegin(env->stack), value_t, link);

    switch (operand->type) {
    case VALUE_TYPE_BOOL:
        switch (expr->type) {
        case EXPRESSION_TYPE_NOT:
            operand->u.bool_value = !operand->u.bool_value;
            break;
        default:
            runtime_error("(%d, %d): unsupport %s(%s)", 
                          expr->line, 
                          expr->column,
                          get_expression_type_string(expr->type),
                          get_value_type_string(operand->type));
        }
        break;

    case VALUE_TYPE_CHAR:
        switch (expr->type) {
        case EXPRESSION_TYPE_PLUS:
            break;
        case EXPRESSION_TYPE_MINUS:
            operand->u.bool_value = -operand->u.bool_value;
            break;
        case EXPRESSION_TYPE_CPL:
            operand->u.bool_value = ~operand->u.bool_value;
            break;
        default:
            runtime_error("(%d, %d): unsupport %s(%s)", 
                          expr->line, 
                          expr->column,
                          get_expression_type_string(expr->type),
                          get_value_type_string(operand->type));
        }
        break;

    case VALUE_TYPE_INT:
        switch (expr->type) {
        case EXPRESSION_TYPE_PLUS:
            break;
        case EXPRESSION_TYPE_MINUS:
            operand->u.int_value = -operand->u.int_value;
            break;
        case EXPRESSION_TYPE_CPL:
            operand->u.int_value = ~operand->u.int_value;
            break;
        default:
            runtime_error("(%d, %d): unsupport %s(%s)", 
                          expr->line, 
                          expr->column,
                          get_expression_type_string(expr->type),
                          get_value_type_string(operand->type));
        }
        break;

    case VALUE_TYPE_LONG:
        switch (expr->type) {
        case EXPRESSION_TYPE_PLUS:
            break;
        case EXPRESSION_TYPE_MINUS:
            operand->u.long_value = -operand->u.long_value;
            break;
        case EXPRESSION_TYPE_CPL:
            operand->u.long_value = ~operand->u.long_value;
            break;
        default:
            runtime_error("(%d, %d): unsupport %s(%s)", 
                          expr->line, 
                          expr->column,
                          get_expression_type_string(expr->type),
                          get_value_type_string(operand->type));
        }
        break;

    case VALUE_TYPE_FLOAT:
        switch (expr->type) {
        case EXPRESSION_TYPE_PLUS:
            break;
        case EXPRESSION_TYPE_MINUS:
            operand->u.float_value = -operand->u.float_value;
            break;
        default:
            runtime_error("(%d, %d): unsupport %s(%s)", 
                          expr->line, 
                          expr->column,
                          get_expression_type_string(expr->type),
                          get_value_type_string(operand->type));
        }
        break;

    case VALUE_TYPE_DOUBLE:
        switch (expr->type) {
        case EXPRESSION_TYPE_PLUS:
            break;
        case EXPRESSION_TYPE_MINUS:
            operand->u.double_value = -operand->u.double_value;
            break;
        default:
            runtime_error("(%d, %d): unsupport %s(%s)", 
                          expr->line, 
                          expr->column,
                          get_expression_type_string(expr->type),
                          get_value_type_string(operand->type));
        }
        break;

    default:
        runtime_error("(%d, %d): unsupport %s(%s)", 
                      expr->line, 
                      expr->column,
                      get_expression_type_string(expr->type),
                      get_value_type_string(operand->type));
        break;
    }
}

static void __evaluator_inc_dec_expression__(environment_t env, expression_t expr, bool toplevel)
{
    value_t operand;

    assert(expr->type == EXPRESSION_TYPE_INC || expr->type == EXPRESSION_TYPE_DEC);

    operand = __evaluator_get_lvalue__(env, expr->u.unary_expr, toplevel);

    switch (operand->type) {
    case VALUE_TYPE_CHAR:
        switch (expr->type) {
        case EXPRESSION_TYPE_INC:
            operand->u.char_value++;
            break;
        case EXPRESSION_TYPE_DEC:
            operand->u.char_value--;
            break;
        default:
            assert(false);
            break;
        }
        break;
    case VALUE_TYPE_INT:
        switch (expr->type) {
        case EXPRESSION_TYPE_INC:
            operand->u.int_value++;
            break;
        case EXPRESSION_TYPE_DEC:
            operand->u.int_value--;
            break;
        default:
            assert(false);
            break;
        }
        break;
    case VALUE_TYPE_LONG:
        switch (expr->type) {
        case EXPRESSION_TYPE_INC:
            operand->u.long_value++;
            break;
        case EXPRESSION_TYPE_DEC:
            operand->u.long_value--;
            break;
        default:
            assert(false);
            break;
        }
        break;
    case VALUE_TYPE_FLOAT:
        switch (expr->type) {
        case EXPRESSION_TYPE_INC:
            operand->u.float_value++;
            break;
        case EXPRESSION_TYPE_DEC:
            operand->u.float_value--;
            break;
        default:
            assert(false);
            break;
        }
        break;
    case VALUE_TYPE_DOUBLE:
        switch (expr->type) {
        case EXPRESSION_TYPE_INC:
            operand->u.double_value++;
            break;
        case EXPRESSION_TYPE_DEC:
            operand->u.double_value--;
            break;
        default:
            assert(false);
            break;
        }
        break;
    default:
        runtime_error("(%d, %d): unsupport %s(%s)", 
                      expr->line, 
                      expr->column,
                      get_expression_type_string(expr->type),
                      get_value_type_string(operand->type));
        break;
    }

    list_push_back(env->stack, value_dup(operand)->link);
}

static void __evaluator_binary_expression__(environment_t env, expression_type_t type, expression_t left_expr, expression_t right_expr, bool toplevel)
{
    value_t left_value;
    value_t right_value;

    evaluator_expression(env, left_expr, toplevel);
    left_value  = list_element(list_rbegin(env->stack), value_t, link);
    evaluator_expression(env, right_expr, toplevel);
    right_value = list_element(list_rbegin(env->stack), value_t, link);

    evaluator_binary_value(env, left_expr->line, left_expr->column, type, left_value, right_value);

    list_erase(env->stack, left_value->link);
    value_free(left_value);

    list_erase(env->stack, right_value->link);
    value_free(right_value);
}

static value_type_t __evaluator_implicit_cast_expression__(value_t left_value, value_t right_value)
{
    if ((left_value->type == VALUE_TYPE_CHAR && right_value->type == VALUE_TYPE_CHAR) ||
        (left_value->type == VALUE_TYPE_INT  && right_value->type == VALUE_TYPE_CHAR) ||
        (left_value->type == VALUE_TYPE_CHAR && right_value->type == VALUE_TYPE_INT)  ||
        (left_value->type == VALUE_TYPE_LONG && right_value->type == VALUE_TYPE_CHAR) ||
        (left_value->type == VALUE_TYPE_CHAR && right_value->type == VALUE_TYPE_LONG)) {
        return VALUE_TYPE_CHAR;

    } else if (left_value->type == VALUE_TYPE_BOOL && right_value->type == VALUE_TYPE_BOOL) {
        return VALUE_TYPE_BOOL;

    } else if (left_value->type == VALUE_TYPE_INT && right_value->type == VALUE_TYPE_INT) {
        return VALUE_TYPE_INT;

    } else if (left_value->type == VALUE_TYPE_FLOAT && right_value->type == VALUE_TYPE_INT) {
        right_value->u.float_value = (float)right_value->u.int_value;
        return VALUE_TYPE_FLOAT;

    } else if (left_value->type == VALUE_TYPE_INT && right_value->type == VALUE_TYPE_FLOAT) {
        left_value->u.float_value = (float)left_value->u.int_value;
        return VALUE_TYPE_FLOAT;

    } else if (left_value->type == VALUE_TYPE_DOUBLE && right_value->type == VALUE_TYPE_INT) {
        right_value->u.double_value = (double)right_value->u.int_value;
        return VALUE_TYPE_DOUBLE;

    } else if (left_value->type == VALUE_TYPE_INT && right_value->type == VALUE_TYPE_DOUBLE) {
        left_value->u.double_value = (double)left_value->u.int_value;
        return VALUE_TYPE_DOUBLE;

    } else if (left_value->type == VALUE_TYPE_LONG && right_value->type == VALUE_TYPE_LONG) {
        return VALUE_TYPE_LONG;

    } else if (left_value->type == VALUE_TYPE_FLOAT && right_value->type == VALUE_TYPE_LONG) {
        right_value->u.float_value = (float)right_value->u.long_value;
        return VALUE_TYPE_FLOAT;

    } else if (left_value->type == VALUE_TYPE_LONG && right_value->type == VALUE_TYPE_FLOAT) {
        left_value->u.float_value = (float)left_value->u.long_value;
        return VALUE_TYPE_FLOAT;

    } else if (left_value->type == VALUE_TYPE_DOUBLE && right_value->type == VALUE_TYPE_LONG) {
        right_value->u.double_value = (double)right_value->u.long_value;
        return VALUE_TYPE_DOUBLE;

    } else if (left_value->type == VALUE_TYPE_LONG && right_value->type == VALUE_TYPE_DOUBLE) {
        left_value->u.double_value = (double)left_value->u.long_value;
        return VALUE_TYPE_DOUBLE;

    } else if (left_value->type == VALUE_TYPE_FLOAT && right_value->type == VALUE_TYPE_FLOAT) {
        return VALUE_TYPE_FLOAT;

    } else if (left_value->type == VALUE_TYPE_DOUBLE && right_value->type == VALUE_TYPE_FLOAT) {
        right_value->u.double_value = (double)right_value->u.float_value;
        return VALUE_TYPE_DOUBLE;

    } else if (left_value->type == VALUE_TYPE_FLOAT && right_value->type == VALUE_TYPE_DOUBLE) {
        left_value->u.double_value = (double)left_value->u.float_value;
        return VALUE_TYPE_DOUBLE;

    } else if (left_value->type == VALUE_TYPE_DOUBLE && right_value->type == VALUE_TYPE_DOUBLE) {
        return VALUE_TYPE_DOUBLE;

    } else if (left_value->type == VALUE_TYPE_STRING && right_value->type == VALUE_TYPE_STRING) {
        return VALUE_TYPE_STRING;

    } else if (left_value->type == VALUE_TYPE_NULL || right_value->type == VALUE_TYPE_NULL) {
        return VALUE_TYPE_NULL;
    }
    
    return VALUE_TYPE_NIL;
}

static void __evaluator_char_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_null(env);

    result = list_element(list_rbegin(env->stack), value_t, link);

    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_CHAR;

    } else if (__is_bit_operator__(type)) {
        result->type = VALUE_TYPE_CHAR;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }

    switch (type) {
    case EXPRESSION_TYPE_BITAND:
        result->u.char_value = left->u.char_value & right->u.char_value;
        break;

    case EXPRESSION_TYPE_BITOR:
        result->u.char_value = left->u.char_value | right->u.char_value;
        break;

    case EXPRESSION_TYPE_XOR:
        result->u.char_value = left->u.char_value ^ right->u.char_value;
        break;

    case EXPRESSION_TYPE_LEFT_SHIFT:
        result->u.char_value = left->u.char_value << right->u.char_value;
        break;

    case EXPRESSION_TYPE_RIGHT_SHIFT:
        result->u.char_value = left->u.char_value >> right->u.char_value;
        break;

    case EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT:
        result->u.char_value = (char)((unsigned char) left->u.char_value >> (unsigned char) right->u.char_value);
        break;

    case EXPRESSION_TYPE_ADD:
        result->u.char_value = left->u.char_value + right->u.char_value;
        break;

    case EXPRESSION_TYPE_SUB:
        result->u.char_value = left->u.char_value - right->u.char_value;
        break;

    case EXPRESSION_TYPE_MUL:
        result->u.char_value = left->u.char_value * right->u.char_value;
        break;

    case EXPRESSION_TYPE_DIV:
        if (right->u.char_value == 0) {
            /* runtime_error("(%d, %d): div by zero", line, column); */
            result->u.char_value = 0;
        } else {
            result->u.char_value = left->u.char_value / right->u.char_value;
        }
        
        break;

    case EXPRESSION_TYPE_MOD:
        if (right->u.char_value == 0) {
            result->u.char_value = 0;
        } else {
            result->u.char_value = left->u.char_value % right->u.char_value;
        }
        break;

    case EXPRESSION_TYPE_GT:
        result->u.bool_value = left->u.char_value > right->u.char_value;
        break;

    case EXPRESSION_TYPE_GEQ:
        result->u.bool_value = left->u.char_value >= right->u.char_value;
        break;

    case EXPRESSION_TYPE_LT:
        result->u.bool_value = left->u.char_value < right->u.char_value;
        break;

    case EXPRESSION_TYPE_LEQ:
        result->u.bool_value = left->u.char_value <= right->u.char_value;
        break;

    case EXPRESSION_TYPE_EQ:
        result->u.bool_value = left->u.char_value == right->u.char_value;
        break;

    case EXPRESSION_TYPE_NEQ:
        result->u.bool_value = left->u.char_value != right->u.char_value;
        break;

    default:
        result->type = VALUE_TYPE_NULL;
        break;
    }
}

static void __evaluator_bool_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_bool(env, false);

    result = list_element(list_rbegin(env->stack), value_t, link);

    if (type == EXPRESSION_TYPE_EQ) {
        result->u.bool_value = left->u.bool_value == right->u.bool_value;

    } else if (type == EXPRESSION_TYPE_NEQ) {
        result->u.bool_value = left->u.bool_value != right->u.bool_value;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)",
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }
}

static void __evaluator_int_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_null(env);

    result = list_element(list_rbegin(env->stack), value_t, link);

    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_INT;

    } else if (__is_bit_operator__(type)) {
        result->type = VALUE_TYPE_INT;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }

    switch (type) {
    case EXPRESSION_TYPE_BITAND:
        result->u.int_value = left->u.int_value & right->u.int_value;
        break;

    case EXPRESSION_TYPE_BITOR:
        result->u.int_value = left->u.int_value | right->u.int_value;
        break;

    case EXPRESSION_TYPE_XOR:
        result->u.int_value = left->u.int_value ^ right->u.int_value;
        break;

    case EXPRESSION_TYPE_LEFT_SHIFT:
        result->u.int_value = left->u.int_value << right->u.int_value;
        break;

    case EXPRESSION_TYPE_RIGHT_SHIFT:
        result->u.int_value = left->u.int_value >> right->u.int_value;
        break;

    case EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT:
        result->u.int_value = (int)((unsigned int)left->u.int_value >> (unsigned int)right->u.int_value);
        break;

    case EXPRESSION_TYPE_ADD:
        result->u.int_value = left->u.int_value + right->u.int_value;
        break;

    case EXPRESSION_TYPE_SUB:
        result->u.int_value = left->u.int_value - right->u.int_value;
        break;

    case EXPRESSION_TYPE_MUL:
        result->u.int_value = left->u.int_value * right->u.int_value;
        break;

    case EXPRESSION_TYPE_DIV:
        if (right->u.int_value == 0) {
            /* runtime_error("(%d, %d): div by zero", line, column); */
            result->u.int_value = 0;
        } else {
            result->u.int_value = left->u.int_value / right->u.int_value;
        }
        break;

    case EXPRESSION_TYPE_MOD:
        if (right->u.int_value == 0) {
            /* runtime_error("(%d, %d): div by zero", line, column); */
            result->u.int_value = 0;
        } else {
            result->u.int_value = left->u.int_value % right->u.int_value;
        }
        break;

    case EXPRESSION_TYPE_GT:
        result->u.bool_value = left->u.int_value > right->u.int_value;
        break;

    case EXPRESSION_TYPE_GEQ:
        result->u.bool_value = left->u.int_value >= right->u.int_value;
        break;

    case EXPRESSION_TYPE_LT:
        result->u.bool_value = left->u.int_value < right->u.int_value;
        break;

    case EXPRESSION_TYPE_LEQ:
        result->u.bool_value = left->u.int_value <= right->u.int_value;
        break;

    case EXPRESSION_TYPE_EQ:
        result->u.bool_value = left->u.int_value == right->u.int_value;
        break;

    case EXPRESSION_TYPE_NEQ:
        result->u.bool_value = left->u.int_value != right->u.int_value;
        break;

    default:
        result->type = VALUE_TYPE_NULL;
        break;
    }
}

static void __evaluator_long_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_null(env);

    result = list_element(list_rbegin(env->stack), value_t, link);

    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_LONG;

    } else if (__is_bit_operator__(type)) {
        result->type = VALUE_TYPE_LONG;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }

    switch (type) {
    case EXPRESSION_TYPE_BITAND:
        result->u.long_value = left->u.long_value & right->u.long_value;
        break;

    case EXPRESSION_TYPE_BITOR:
        result->u.long_value = left->u.long_value | right->u.long_value;
        break;

    case EXPRESSION_TYPE_XOR:
        result->u.long_value = left->u.long_value ^ right->u.long_value;
        break;

    case EXPRESSION_TYPE_LEFT_SHIFT:
        result->u.long_value = left->u.long_value << right->u.long_value;
        break;

    case EXPRESSION_TYPE_RIGHT_SHIFT:
        result->u.long_value = left->u.long_value >> right->u.long_value;
        break;

    case EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT:
        result->u.long_value = (long)((unsigned long)left->u.long_value >> (unsigned long)right->u.long_value);
        break;

    case EXPRESSION_TYPE_ADD:
        result->u.long_value = left->u.long_value + right->u.long_value;
        break;

    case EXPRESSION_TYPE_SUB:
        result->u.long_value = left->u.long_value - right->u.long_value;
        break;

    case EXPRESSION_TYPE_MUL:
        result->u.long_value = left->u.long_value * right->u.long_value;
        break;

    case EXPRESSION_TYPE_DIV:
        if (right->u.long_value == 0) {
            /* runtime_error("(%d, %d): div by zero", line, column); */
            result->u.long_value = 0;
        } else {
            result->u.long_value = left->u.long_value / right->u.long_value;
        }
        break;

    case EXPRESSION_TYPE_MOD:
        if (right->u.long_value == 0) {
            /* runtime_error("(%d, %d): div by zero", line, column); */
            result->u.long_value = 0;
        } else {
            result->u.long_value = left->u.long_value % right->u.long_value;
        }
        break;

    case EXPRESSION_TYPE_GT:
        result->u.bool_value = left->u.long_value > right->u.long_value;
        break;

    case EXPRESSION_TYPE_GEQ:
        result->u.bool_value = left->u.long_value >= right->u.long_value;
        break;

    case EXPRESSION_TYPE_LT:
        result->u.bool_value = left->u.long_value < right->u.long_value;
        break;

    case EXPRESSION_TYPE_LEQ:
        result->u.bool_value = left->u.long_value <= right->u.long_value;
        break;

    case EXPRESSION_TYPE_EQ:
        result->u.bool_value = left->u.long_value == right->u.long_value;
        break;

    case EXPRESSION_TYPE_NEQ:
        result->u.bool_value = left->u.long_value != right->u.long_value;
        break;

    default:
        result->type = VALUE_TYPE_NULL;
        break;
    }
}

static void __evaluator_float_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_null(env);

    result = list_element(list_rbegin(env->stack), value_t, link);

    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_FLOAT;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }

    switch (type) {
    case EXPRESSION_TYPE_ADD:
        result->u.float_value = left->u.float_value + right->u.float_value;
        break;

    case EXPRESSION_TYPE_SUB:
        result->u.float_value = left->u.float_value - right->u.float_value;
        break;

    case EXPRESSION_TYPE_MUL:
        result->u.float_value = left->u.float_value * right->u.float_value;
        break;

    case EXPRESSION_TYPE_DIV:
        result->u.float_value = left->u.float_value / right->u.float_value;
        break;

    case EXPRESSION_TYPE_MOD:
        result->u.float_value =(float)fmod((double)left->u.float_value, (double)right->u.float_value);
        break;

    case EXPRESSION_TYPE_GT:
        result->u.bool_value = left->u.float_value > right->u.float_value;
        break;

    case EXPRESSION_TYPE_GEQ:
        result->u.bool_value = left->u.float_value >= right->u.float_value;
        break;

    case EXPRESSION_TYPE_LT:
        result->u.bool_value = left->u.float_value < right->u.float_value;
        break;

    case EXPRESSION_TYPE_LEQ:
        result->u.bool_value = left->u.float_value <= right->u.float_value;
        break;

    case EXPRESSION_TYPE_EQ:
        result->u.bool_value = left->u.float_value == right->u.float_value;
        break;

    case EXPRESSION_TYPE_NEQ:
        result->u.bool_value = left->u.float_value != right->u.float_value;
        break;

    default:
        result->type = VALUE_TYPE_NULL;
        break;
    }
}

static void __evaluator_double_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_null(env);

    result = list_element(list_rbegin(env->stack), value_t, link);

    if (__is_math_operator__(type)) {
        result->type = VALUE_TYPE_DOUBLE;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }

    switch (type) {
    case EXPRESSION_TYPE_ADD:
        result->u.double_value = left->u.double_value + right->u.double_value;
        break;

    case EXPRESSION_TYPE_SUB:
        result->u.double_value = left->u.double_value - right->u.double_value;
        break;

    case EXPRESSION_TYPE_MUL:
        result->u.double_value = left->u.double_value * right->u.double_value;
        break;

    case EXPRESSION_TYPE_DIV:
        result->u.double_value = left->u.double_value / right->u.double_value;
        break;

    case EXPRESSION_TYPE_MOD:
        result->u.double_value = fmod(left->u.double_value, right->u.double_value);
        break;

    case EXPRESSION_TYPE_GT:
        result->u.bool_value = left->u.double_value > right->u.double_value;
        break;

    case EXPRESSION_TYPE_GEQ:
        result->u.bool_value = left->u.double_value >= right->u.double_value;
        break;

    case EXPRESSION_TYPE_LT:
        result->u.bool_value = left->u.double_value < right->u.double_value;
        break;

    case EXPRESSION_TYPE_LEQ:
        result->u.bool_value = left->u.double_value <= right->u.double_value;
        break;

    case EXPRESSION_TYPE_EQ:
        result->u.bool_value = left->u.double_value == right->u.double_value;
        break;

    case EXPRESSION_TYPE_NEQ:
        result->u.bool_value = left->u.double_value != right->u.double_value;
        break;

    default:
        result->type = VALUE_TYPE_NULL;
        break;
    }
}

static void __evaluator_string_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_null(env);
    
    result = list_element(list_rbegin(env->stack), value_t, link);

    if (type == EXPRESSION_TYPE_ADD) {
        result->u.object_value           = heap_alloc_string_n(env, cstring_length(left->u.object_value->u.string) + cstring_length(right->u.object_value->u.string) + 10);
        result->u.object_value->u.string = cstring_cat(result->u.object_value->u.string, left->u.object_value->u.string);
        result->type                     = VALUE_TYPE_STRING;

    } else if (__is_compare_operator__(type)) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }

    switch (type) {
    case EXPRESSION_TYPE_ADD:
        result->u.object_value->u.string = cstring_cat(result->u.object_value->u.string, right->u.object_value->u.string);
        break;

    case EXPRESSION_TYPE_GT:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) > 0;
        break;

    case EXPRESSION_TYPE_GEQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) >= 0;
        break;

    case EXPRESSION_TYPE_LT:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) < 0;
        break;

    case EXPRESSION_TYPE_LEQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) <= 0;
        break;

    case EXPRESSION_TYPE_EQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) == 0;
        break;

    case EXPRESSION_TYPE_NEQ:
        result->u.bool_value = cstring_cmp(left->u.object_value->u.string, right->u.object_value->u.string) != 0;
        break;

    default:
        result->type = VALUE_TYPE_NULL;
        break;
    }
}

static void __evaluator_null_binary_expression__(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right)
{
    value_t result;

    environment_push_null(env);

    result = list_element(list_rbegin(env->stack), value_t, link);

    if (type == EXPRESSION_TYPE_EQ || type == EXPRESSION_TYPE_NEQ) {
        result->type = VALUE_TYPE_BOOL;

    } else {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      line,
                      column,
                      get_value_type_string(left->type),
                      get_expression_type_string(type),
                      get_value_type_string(right->type));
    }

    switch (type) {
    case EXPRESSION_TYPE_EQ:
        result->u.bool_value = left->type == VALUE_TYPE_NULL && left->type == VALUE_TYPE_NULL;
        break;
    case EXPRESSION_TYPE_NEQ:
        result->u.bool_value = !(left->type == VALUE_TYPE_NULL && left->type == VALUE_TYPE_NULL);
        break;
    default:
        result->type = VALUE_TYPE_NULL;
        break;
    }
}

static void __evaluator_logic_binary_expression__(environment_t env, expression_type_t type, expression_t left_expr, expression_t right_expr, bool toplevel)
{
    value_t lvalue = NULL;
    value_t rvalue = NULL;

    evaluator_expression(env, left_expr, toplevel);

    lvalue = list_element(list_rbegin(env->stack), value_t, link);
    list_pop_back(env->stack);

    if (lvalue->type != VALUE_TYPE_BOOL) {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s", 
                      left_expr->line,
                      left_expr->column,
                      get_value_type_string(lvalue->type),
                      get_expression_type_string(type));
    }

    switch (type) {
    case EXPRESSION_TYPE_AND:
        if (lvalue->u.bool_value == false) {
            goto eval_success;
        }
        break;

    case EXPRESSION_TYPE_OR:
        if (lvalue->u.bool_value == true) {
            environment_push_bool(env, true);
            goto eval_success;
        }
        break;

    default:
        assert(false);
        break;
    }

    evaluator_expression(env, right_expr, toplevel);

    rvalue = list_element(list_rbegin(env->stack), value_t, link);
    list_pop_back(env->stack);

    if (rvalue->type != VALUE_TYPE_BOOL) {
        runtime_error("(%d, %d): unsupported operand for : type(%s) %s type(%s)", 
                      left_expr->line,
                      left_expr->column,
                      get_value_type_string(lvalue->type),
                      get_expression_type_string(type),
                      get_value_type_string(rvalue->type));
    }

    environment_push_bool(env, rvalue->u.bool_value);

eval_success:
    if (lvalue) {
        value_free(lvalue);
    }
    if (rvalue) {
        value_free(rvalue);
    }
}

const char* get_expression_type_string(expression_type_t type)
{
    switch (type) {
    case EXPRESSION_TYPE_CHAR:
        return "char";
    case EXPRESSION_TYPE_BOOL:
        return "bool";
    case EXPRESSION_TYPE_INT:
        return "int";
    case EXPRESSION_TYPE_LONG:
        return "long";
    case EXPRESSION_TYPE_FLOAT:
        return "float";
    case EXPRESSION_TYPE_DOUBLE:
        return "double";
    case EXPRESSION_TYPE_STRING:
        return "string";
    case EXPRESSION_TYPE_NULL:
        return "null";
    case EXPRESSION_TYPE_IDENTIFIER:
        return "identifier";
    case EXPRESSION_TYPE_ASSIGN:
        return "=";
    case EXPRESSION_TYPE_ADD_ASSIGN:
        return "+=";
    case EXPRESSION_TYPE_SUB_ASSIGN:
        return "-=";
    case EXPRESSION_TYPE_MUL_ASSIGN:
        return "*=";
    case EXPRESSION_TYPE_DIV_ASSIGN:
        return "/=";
    case EXPRESSION_TYPE_MOD_ASSIGN:
        return "%=";
    case EXPRESSION_TYPE_CALL:
        return "function call";
    case EXPRESSION_TYPE_PLUS:
        return "+";
    case EXPRESSION_TYPE_MINUS:
        return "-";
    case EXPRESSION_TYPE_NOT:
        return "!";
    case EXPRESSION_TYPE_INC:
        return "++";
    case EXPRESSION_TYPE_DEC:
        return "--";
    case EXPRESSION_TYPE_BITAND:
        return "&";
    case EXPRESSION_TYPE_BITOR:
        return "|";
    case EXPRESSION_TYPE_XOR:
        return "^";
    case EXPRESSION_TYPE_LEFT_SHIFT:
        return "<<";
    case EXPRESSION_TYPE_RIGHT_SHIFT:
        return ">>";
    case EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT:
        return ">>>";
    case EXPRESSION_TYPE_MUL:
        return "*";
    case EXPRESSION_TYPE_DIV:
        return "/";
    case EXPRESSION_TYPE_MOD:
        return "%";
    case EXPRESSION_TYPE_ADD:
        return "+";
    case EXPRESSION_TYPE_SUB:
        return "-";
    case EXPRESSION_TYPE_GT:
        return ">";
    case EXPRESSION_TYPE_GEQ:
        return ">=";
    case EXPRESSION_TYPE_LT:
        return "<";
    case EXPRESSION_TYPE_LEQ:
        return "<=";
    case EXPRESSION_TYPE_EQ:
        return "==";
    case EXPRESSION_TYPE_NEQ:
        return "!=";
    case EXPRESSION_TYPE_AND:
        return "&&";
    case EXPRESSION_TYPE_OR:
        return "||";
    default:
        return "unknown";
    }

    return "unknown";
}

const char* get_value_type_string(value_type_t type)
{
    switch (type) {
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
    case VALUE_TYPE_FUNCTION:
        return "function";
    case VALUE_TYPE_REFERENCE:
        return "null";
    case VALUE_TYPE_NULL:
        return "null";
    default:
        return "unknown";
    }

    return "unknown";
}