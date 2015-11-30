

#include "expression.h"
#include "statement.h"
#include "alloc.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static expression_t __expression_new__(expression_type_t expr_type, long line, long column)
{
    expression_t expr = mem_alloc(sizeof(struct expression_s));
    if (!expr) {
        return NULL;
    }
    
    expr->type   = expr_type;
    expr->line   = line;
    expr->column = column;

    return expr;
}

const char* expression_type_string(expression_type_t expr_type)
{
    switch (expr_type) {
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

expression_t expression_new_literal(expression_type_t type, token_t tok)
{
    expression_t expr;

    assert(tok->value == TOKEN_VALUE_TRUE           ||
           tok->value == TOKEN_VALUE_FALSE          || 
           tok->value == TOKEN_VALUE_NULL           ||
           tok->value == TOKEN_VALUE_LITERAL_CHAR   || 
           tok->value == TOKEN_VALUE_LITERAL_INT    ||
           tok->value == TOKEN_VALUE_LITERAL_LONG   ||
           tok->value == TOKEN_VALUE_LITERAL_FLOAT  ||
           tok->value == TOKEN_VALUE_LITERAL_DOUBLE ||
           tok->value == TOKEN_VALUE_LITERAL_STRING);

    expr = __expression_new__(type, tok->line, tok->column);

    switch (type) {
    case EXPRESSION_TYPE_BOOL:
        if (tok->value == TOKEN_VALUE_TRUE) {
            expr->u.bool_expr = true;
        } else if (tok->value == TOKEN_VALUE_FALSE) {
            expr->u.bool_expr = false;
        }
        break;

    case EXPRESSION_TYPE_CHAR:
        sscanf(tok->token, "%c", &expr->u.char_expr);
        break;

    case EXPRESSION_TYPE_INT:
        sscanf(tok->token, "%d", &expr->u.int_expr);
        break;

    case EXPRESSION_TYPE_LONG:
        sscanf(tok->token, "%ld", &expr->u.long_expr);
        break;

    case EXPRESSION_TYPE_FLOAT:
        sscanf(tok->token, "%f", &expr->u.float_expr);
        break;

    case EXPRESSION_TYPE_DOUBLE:
        sscanf(tok->token, "%lf", &expr->u.double_expr);
        break;

    case EXPRESSION_TYPE_STRING:
        expr->u.string_expr = cstring_dup(tok->token);
        break;

    default:
        assert(false);
        break;
    }

    return expr;
}

expression_t expression_new_identifier(long line, long column, cstring_t identifier)
{
    expression_t expr = __expression_new__(EXPRESSION_TYPE_IDENTIFIER, line, column);

    assert(!cstring_is_empty(identifier));

    expr->u.identifier_expr = identifier;

    return expr;
}

expression_t expression_new_ignore(long line, long column)
{
    return __expression_new__(EXPRESSION_TYPE_IGNORE, line, column);
}

expression_t expression_new_assign(long line, long column, list_t lvalue_exprs, int lvalue_exprs_total,  list_t rvalue_exprs, int rvalue_exprs_total)
{
    expression_t expr;
    expression_assign_t assign_expr;

    expr = __expression_new__(EXPRESSION_TYPE_ASSIGN, line, column);

    assign_expr = (expression_assign_t) mem_alloc(sizeof(struct expression_assign_s));
    if (!assign_expr) {
        return NULL;
    }

    expr->u.assign_expr               = assign_expr;
    expr->u.assign_expr->lvalue_exprs = lvalue_exprs;
    expr->u.assign_expr->rvalue_exprs = rvalue_exprs;
    expr->u.assign_expr->lvalue_exprs_total = lvalue_exprs_total;
    expr->u.assign_expr->rvalue_exprs_total = rvalue_exprs_total;

    return expr;
}

expression_t expression_new_component_assign(long line, long column, expression_type_t component_assign_type, expression_t lvalue_expr, expression_t rvalue_expr)
{
    expression_t expr;
    expression_component_assign_t assign_expr;

    assert(component_assign_type == EXPRESSION_TYPE_ADD_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_SUB_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_MUL_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_DIV_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_MOD_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_BITAND_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_BITOR_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_XOR_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_LEFI_SHIFT_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_RIGHT_SHIFT_ASSIGN ||
           component_assign_type == EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT_ASSIGN);

    expr = __expression_new__(component_assign_type, line, column);

    assign_expr = (expression_component_assign_t) mem_alloc(sizeof(struct expression_component_assign_s));
    if (!assign_expr) {
        return NULL;
    }

    expr->u.component_assign_expr              = assign_expr;
    expr->u.component_assign_expr->lvalue_expr = lvalue_expr;
    expr->u.component_assign_expr->rvalue_expr = rvalue_expr;

    return expr;
}

expression_t expression_new_binary(long line, long column, expression_type_t binary_expr_type, expression_t left, expression_t right)
{
    expression_t expr;
    expression_binary_t binary_expr;

    assert(binary_expr_type == EXPRESSION_TYPE_OR || 
           binary_expr_type == EXPRESSION_TYPE_AND || 
           binary_expr_type == EXPRESSION_TYPE_EQ || 
           binary_expr_type == EXPRESSION_TYPE_NEQ || 
           binary_expr_type == EXPRESSION_TYPE_GT ||
           binary_expr_type == EXPRESSION_TYPE_GEQ ||
           binary_expr_type == EXPRESSION_TYPE_LT || 
           binary_expr_type == EXPRESSION_TYPE_LEQ ||
           binary_expr_type == EXPRESSION_TYPE_ADD || 
           binary_expr_type == EXPRESSION_TYPE_SUB || 
           binary_expr_type == EXPRESSION_TYPE_MUL ||
           binary_expr_type == EXPRESSION_TYPE_DIV ||
           binary_expr_type == EXPRESSION_TYPE_MOD ||
           binary_expr_type == EXPRESSION_TYPE_BITAND ||
           binary_expr_type == EXPRESSION_TYPE_BITOR || 
           binary_expr_type == EXPRESSION_TYPE_XOR ||
           binary_expr_type == EXPRESSION_TYPE_LEFT_SHIFT ||
           binary_expr_type == EXPRESSION_TYPE_RIGHT_SHIFT ||
           binary_expr_type == EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT);

    assert(left != NULL);
    assert(right != NULL);

    expr = __expression_new__(binary_expr_type, line, column);

    binary_expr = (expression_binary_t) mem_alloc(sizeof (struct expression_binary_s));
    if (!binary_expr) {
        return NULL;
    }

    expr->u.binary_expr        = binary_expr;
    expr->u.binary_expr->left  = left;
    expr->u.binary_expr->right = right;

    return expr;
}

expression_t expression_new_unary(long line, long column, expression_type_t unary_expr_type, expression_t expression)
{
    expression_t expr;

    assert(unary_expr_type == EXPRESSION_TYPE_PLUS || 
           unary_expr_type == EXPRESSION_TYPE_MINUS ||
           unary_expr_type == EXPRESSION_TYPE_NOT || 
           unary_expr_type == TOKEN_VALUE_CPL);

    assert(expression != NULL);

    expr = __expression_new__(unary_expr_type, line, column);

    expr->u.unary_expr = expression;

    return expr;
}

expression_t expression_new_incdec(long line, long column, expression_type_t type, expression_t lvalue_expr)
{
    expression_t expr;

    assert(type == EXPRESSION_TYPE_INC || 
           type == EXPRESSION_TYPE_DEC);

    assert(lvalue_expr != NULL);

    expr = __expression_new__(type, line, column);

    expr->u.incdec_expr = lvalue_expr;

    return expr;
}

expression_t expression_new_function(long line, long column, cstring_t name, list_t parameters, list_t block)
{
    expression_t expr;
    expression_function_t function_expr;

    expr = __expression_new__(EXPRESSION_TYPE_FUNCTION, line, column);

    function_expr = (expression_function_t) mem_alloc(sizeof (struct expression_function_s));
    if (!function_expr) {
        return NULL;
    }

    expr->u.function_expr             = function_expr;
    expr->u.function_expr->name       = name;
    expr->u.function_expr->parameters = parameters;
    expr->u.function_expr->block      = block;

    return expr;
}

expression_t expression_new_call(long line, long column, expression_t function_expr, list_t args)
{
    expression_t expr;
    expression_call_t call_expr;

    assert(function_expr != NULL);

    expr = __expression_new__(EXPRESSION_TYPE_CALL, line, column);

    call_expr = (expression_call_t) mem_alloc(sizeof (struct expression_call_s));
    if (!call_expr) {
        return NULL;
    }

    expr->u.call_expr                = call_expr;
    expr->u.call_expr->function_expr = function_expr;
    expr->u.call_expr->args          = args;

    return expr;
}

expression_t expression_new_list(long line, long column, list_t exprs)
{
    expression_t expr;
    
    expr = __expression_new__(EXPRESSION_TYPE_LIST, line, column);

    expr->u.expressions_expr = exprs;

    return expr;
}

expression_t expression_new_array_generate(long line, long column, list_t elements)
{
    expression_t expr;

    expr = __expression_new__(EXPRESSION_TYPE_ARRAY_GENERATE, line, column);

    expr->u.array_generate_expr = elements;

    return expr;
}

expression_t expression_new_array_push(long line, long column, expression_t array_expr, expression_t elem_expr)
{
    expression_t expr;
    expression_array_push_t array_append;

    expr = __expression_new__(EXPRESSION_TYPE_ARRAY_PUSH, line, column);

    array_append = (expression_array_push_t) mem_alloc(sizeof(struct expression_array_push_s));
    if (!array_append) {
        return NULL;
    }

    expr->u.array_push_expr             = array_append;
    expr->u.array_push_expr->array_expr = array_expr;
    expr->u.array_push_expr->elem_expr  = elem_expr;

    return expr;
}

expression_t expression_new_array_pop(long line, long column, expression_t array_expr, expression_t lvalue_expr)
{
    expression_t expr;
    expression_array_pop_t array_pop;

    expr = __expression_new__(EXPRESSION_TYPE_ARRAY_POP, line, column);

    array_pop = (expression_array_pop_t) mem_alloc(sizeof(struct expression_array_pop_s));
    if (!array_pop) {
        return NULL;
    }

    expr->u.array_pop_expr              = array_pop;
    expr->u.array_pop_expr->array_expr  = array_expr;
    expr->u.array_pop_expr->lvalue_expr = lvalue_expr;

    return expr;
}

expression_table_pair_t expression_new_table_pair(cstring_t name, expression_t expr)
{
    expression_table_pair_t pair = (expression_table_pair_t) mem_alloc(sizeof(struct expression_table_pair_s));
    if (!pair) {
        return NULL;
    }

    pair->member_name = name;
    pair->member_expr = expr;

    return pair;
}

void expression_free_table_pair(expression_table_pair_t pair)
{
    cstring_free(pair->member_name);
    expression_free(pair->member_expr);
    mem_free(pair);
}

expression_t expression_new_table_generate(long line, long column, list_t members)
{
    expression_t expr;

    expr = __expression_new__(EXPRESSION_TYPE_TABLE_GENERATE, line, column);

    expr->u.table_generate_expr = members;

    return expr;
}

expression_t expression_new_table_dot_member(long line, long column, expression_t table, cstring_t member_name)
{
    expression_t expr;
    expression_table_dot_member_t dot_member;

    expr = __expression_new__(EXPRESSION_TYPE_TABLE_DOT_MEMBER, line, column);

    dot_member = (expression_table_dot_member_t) mem_alloc(sizeof (struct expression_table_dot_member_s));
    if (!dot_member) {
        return NULL;
    }

    expr->u.table_dot_member_expr              = dot_member;
    expr->u.table_dot_member_expr->table       = table;
    expr->u.table_dot_member_expr->member_name = member_name;

    return expr;
}

expression_t expression_new_index(long line, long column, expression_t dict, expression_t index)
{
    expression_t expr;
    expression_index_t index_expr;

    expr = __expression_new__(EXPRESSION_TYPE_INDEX, line, column);

    index_expr = (expression_index_t) mem_alloc(sizeof (struct expression_index_s));
    if (!index_expr) {
        return NULL;
    }

    expr->u.index_expr        = index_expr;
    expr->u.index_expr->dict  = dict;
    expr->u.index_expr->index = index;

    return expr;
}

void expression_free(expression_t expr)
{
    list_iter_t iter = NULL, next_iter = NULL;

    switch (expr->type) {
    case EXPRESSION_TYPE_STRING:
        cstring_free(expr->u.string_expr);
        break;

    case EXPRESSION_TYPE_IDENTIFIER:
        cstring_free(expr->u.identifier_expr);
        break;

    case EXPRESSION_TYPE_FUNCTION:
        cstring_free(expr->u.function_expr->name);

        list_safe_for_each(expr->u.function_expr->parameters, iter, next_iter) {
            expression_function_parameter_t parameter;

            list_erase(expr->u.function_expr->parameters, *iter);

            parameter = list_element(iter, expression_function_parameter_t, link);

            cstring_free(parameter->name);

            mem_free(parameter);
        }

        list_safe_for_each(expr->u.function_expr->block, iter, next_iter) {
            list_erase(expr->u.function_expr->block, *iter);
            statement_free(list_element(iter, statement_t, link));
        }

        mem_free(expr->u.function_expr);
        break;

    case EXPRESSION_TYPE_ASSIGN:
        list_safe_for_each(expr->u.assign_expr->lvalue_exprs, iter, next_iter) {
            list_erase(expr->u.assign_expr->lvalue_exprs, *iter);
            expression_free(list_element(iter, expression_t, link));
        }
        
        list_safe_for_each(expr->u.assign_expr->rvalue_exprs, iter, next_iter) {
            list_erase(expr->u.assign_expr->rvalue_exprs, *iter);
            expression_free(list_element(iter, expression_t, link));
        }
        
        mem_free(expr->u.assign_expr);
        break;

    case EXPRESSION_TYPE_ADD_ASSIGN:
    case EXPRESSION_TYPE_SUB_ASSIGN:
    case EXPRESSION_TYPE_MUL_ASSIGN:
    case EXPRESSION_TYPE_DIV_ASSIGN:
    case EXPRESSION_TYPE_MOD_ASSIGN:
        expression_free(expr->u.component_assign_expr->lvalue_expr);
        expression_free(expr->u.component_assign_expr->rvalue_expr);
        mem_free(expr->u.component_assign_expr);
        break;

    case EXPRESSION_TYPE_CALL:
        expression_free(expr->u.call_expr->function_expr);
        list_safe_for_each(expr->u.call_expr->args, iter, next_iter) {
            list_erase(expr->u.call_expr->args, *iter);
            expression_free(list_element(iter, expression_t, link));
        }
        mem_free(expr->u.call_expr);
        break;

    case EXPRESSION_TYPE_PLUS:
    case EXPRESSION_TYPE_MINUS:
    case EXPRESSION_TYPE_NOT:
    case EXPRESSION_TYPE_CPL:
        expression_free(expr->u.unary_expr);
        break;

    case EXPRESSION_TYPE_INC:
    case EXPRESSION_TYPE_DEC:
        expression_free(expr->u.incdec_expr);
        break;

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
    case EXPRESSION_TYPE_AND:
    case EXPRESSION_TYPE_OR:
        expression_free(expr->u.binary_expr->left);
        expression_free(expr->u.binary_expr->right);
        mem_free(expr->u.binary_expr);
        break;

    case EXPRESSION_TYPE_LIST:
        list_safe_for_each(expr->u.expressions_expr, iter, next_iter) {
            list_erase(expr->u.expressions_expr, *iter);
            expression_free(list_element(iter, expression_t, link));
        }
        break;

    case EXPRESSION_TYPE_ARRAY_GENERATE:
        list_safe_for_each(expr->u.expressions_expr, iter, next_iter) {
            list_erase(expr->u.expressions_expr, *iter);
            expression_free(list_element(iter, expression_t, link));
        }
        break;

    case EXPRESSION_TYPE_TABLE_GENERATE:
        list_safe_for_each(expr->u.expressions_expr, iter, next_iter) {
            list_erase(expr->u.expressions_expr, *iter);
            expression_free_table_pair(list_element(iter, expression_table_pair_t, link));
        }
        break;

    case EXPRESSION_TYPE_ARRAY_PUSH:
        expression_free(expr->u.array_push_expr->array_expr);
        expression_free(expr->u.array_push_expr->elem_expr);
        mem_free(expr->u.array_push_expr);
        break;

    case EXPRESSION_TYPE_TABLE_DOT_MEMBER:
        expression_free(expr->u.table_dot_member_expr->table);
        cstring_free(expr->u.table_dot_member_expr->member_name);
        mem_free(expr->u.table_dot_member_expr);
        break;

    case EXPRESSION_TYPE_INDEX:
        expression_free(expr->u.index_expr->dict);
        expression_free(expr->u.index_expr->index);
        mem_free(expr->u.index_expr);
        break;

    default:
        break;
    }

    mem_free(expr);
}