

#include "statement.h"
#include "alloc.h"

statement_elif_t statement_new_elif(expression_t condition, list_t block)
{
    statement_elif_t elif = (statement_elif_t) mem_alloc(sizeof(struct statement_elif_s));

    elif->condition = condition;
    elif->block     = block;

    return elif;
}

void statement_free_elif(statement_elif_t elif)
{
    list_iter_t iter, next_iter;

    expression_free(elif->condition);
    
    list_safe_for_each(elif->block, iter, next_iter) {
        list_erase(elif->block, *iter);
        statement_free(list_element(iter, statement_t, link));
    }

    mem_free(elif);
}

statement_t __statement_new__(statement_type_t type, long line, long column)
{
    statement_t stmt = (statement_t) mem_alloc(sizeof(struct statement_s));
    if (!stmt) {
        return NULL;
    }

    stmt->type   = type;
    stmt->line   = line;
    stmt->column = column;

    return stmt;
}

statement_t statement_new_require(long line, long column, cstring_t package_name)
{
    statement_t stmt = __statement_new__(STATEMENT_TYPE_REQUIRE, line, column);

    stmt->u.package_name = package_name;

    return stmt;
}

statement_t statement_new_expression(long line, long column, expression_t expr)
{
    statement_t stmt = __statement_new__(STATEMENT_TYPE_EXPRESSION, line, column);

    stmt->u.expr = expr;

    return stmt;
}

statement_t statement_new_if(long line, long column, expression_t condition, list_t if_block, list_t elifs, list_t else_block)
{
    statement_t stmt;
    statement_if_t if_stmt;

    stmt = __statement_new__(STATEMENT_TYPE_IF, line, column);

    if_stmt = (statement_if_t) mem_alloc(sizeof(struct statement_if_s));
    if (!if_stmt) {
        return NULL;
    }

    stmt->u.if_stmt             = if_stmt;
    stmt->u.if_stmt->condition  = condition;
    stmt->u.if_stmt->if_block   = if_block;
    stmt->u.if_stmt->elifs      = elifs;
    stmt->u.if_stmt->else_block = else_block;

    return stmt;
}

statement_t statement_new_while(long line, long column, expression_t condition, list_t block)
{
    statement_t stmt;
    statement_while_t while_stmt;

    stmt = __statement_new__(STATEMENT_TYPE_WHILE, line, column);

    while_stmt = (statement_while_t) mem_alloc(sizeof(struct statement_while_s));
    if (!while_stmt) {
        return NULL;
    }

    stmt->u.while_stmt            = while_stmt;
    stmt->u.while_stmt->condition = condition;
    stmt->u.while_stmt->block     = block;

    return stmt;
}

void statement_free(statement_t stmt)
{
    list_iter_t iter, next_iter;

    switch (stmt->type) {
    case STATEMENT_TYPE_REQUIRE:
        cstring_free(stmt->u.package_name);
        break;

    case STATEMENT_TYPE_EXPRESSION:
        expression_free(stmt->u.expr);
        break;

    case STATEMENT_TYPE_IF:
        expression_free(stmt->u.if_stmt->condition);

        list_safe_for_each(stmt->u.if_stmt->if_block, iter, next_iter) {
            list_erase(stmt->u.if_stmt->if_block, *iter);
            statement_free(list_element(iter, statement_t, link));
        }

        list_safe_for_each(stmt->u.if_stmt->elifs, iter, next_iter) {
            statement_free_elif(list_element(iter, statement_elif_t, link));
        }

        list_safe_for_each(stmt->u.if_stmt->else_block, iter, next_iter) {
            list_erase(stmt->u.if_stmt->else_block, *iter);
            statement_free(list_element(iter, statement_t, link));
        }
        
        mem_free(stmt->u.if_stmt);
        break;

    case STATEMENT_TYPE_WHILE:
        expression_free(stmt->u.while_stmt->condition);
        list_safe_for_each(stmt->u.while_stmt->block, iter, next_iter) {
            list_erase(stmt->u.while_stmt->block, *iter);
            statement_free(list_element(iter, statement_t, link));
        }
        mem_free(stmt->u.while_stmt);
        break;

    default:
        break;
    }

    mem_free(stmt);
}