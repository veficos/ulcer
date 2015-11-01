

#include "stmt.h"
#include "alloc.h"

static stmt_t __stmt_new__(stmt_type_t type, long line, long column)
{
    stmt_t stmt = heap_alloc(sizeof(struct stmt_s));
    if (!stmt) {
        return NULL;
    }

    stmt->type   = type;
    stmt->line   = line;
    stmt->column = column;

    return stmt;
}

stmt_t stmt_new_expr(long line, long column, expr_t expr)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_EXPR, line, column);
    if (!stmt) {
        return NULL;
    }

    stmt->u.expr = expr;

    return stmt;
}

stmt_t stmt_new_break(long line, long column)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_BREAK, line, column);
    if (!stmt) {
        return NULL;
    }

    return stmt;
}

stmt_t stmt_new_continue(long line, long column)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_CONTINUE, line, column);
    if (!stmt) {
        return NULL;
    }

    return stmt;
}

stmt_t stmt_new_return(long line, long column, expr_t return_value)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_RETURN, line, column);
    if (!stmt) {
        return NULL;
    }

    stmt->u.return_expr = return_value;

    return stmt;
}

stmt_t stmt_new_global(long line, long column)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_GLOBAL, line, column);
    if (!stmt) {
        return NULL;
    }

    list_init(stmt->u.stmt_global);

    return stmt;
}

stmt_t stmt_new_if(long line, long column, expr_t condition, stmt_t if_block)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_IF, line, column);
    if (!stmt) {
        return NULL;
    }

    stmt->u.stmt_if.condition  = condition;
    stmt->u.stmt_if.if_block   = if_block;
    stmt->u.stmt_if.else_block = NULL;

    list_init(stmt->u.stmt_if.elifs);

    return stmt;
}

stmt_t stmt_new_elif(long line, long column, expr_t condition, stmt_t block)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_ELIF, line, column);
    if (!stmt) {
        return NULL;
    }
    
    stmt->u.stmt_elif.condition = condition;
    stmt->u.stmt_elif.block     = block;

    return stmt;
}

stmt_t stmt_new_while(long line, long column, expr_t condition, stmt_t block)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_WHILE, line, column);
    if (!stmt) {
        return NULL;
    }

    stmt->u.stmt_while.condition = condition;
    stmt->u.stmt_while.block     = block;

    return stmt;
}

stmt_t stmt_new_for(long line, long column, expr_t init, expr_t condition, expr_t post, stmt_t block)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_FOR, line, column);
    if (!stmt) {
        return NULL;
    }

    stmt->u.stmt_for.init       = init;
    stmt->u.stmt_for.condition  = condition;
    stmt->u.stmt_for.post       = post;
    stmt->u.stmt_for.block      = block;

    return stmt;
}

stmt_t stmt_new_block(long line, long column)
{
    stmt_t stmt = __stmt_new__(STMT_TYPE_BLOCK, line, column);
    if (!stmt) {
        return NULL;
    }

    list_init(stmt->u.block);

    return stmt;
}

void stmt_free(stmt_t stmt)
{
    list_iter_t iter, next_iter;

    switch (stmt->type) {
    case STMT_TYPE_EXPR:
        expr_free(stmt->u.expr);
        break;

    case STMT_TYPE_BREAK:
    case STMT_TYPE_CONTINUE:
        break;

    case STMT_TYPE_RETURN:
        expr_free(stmt->u.return_expr);
        break;

    case STMT_TYPE_BLOCK:
        list_safe_for_each(stmt->u.block, iter, next_iter) {
            list_erase(*iter);
            stmt_free(list_element(iter, stmt_t, link));
        }
        break;

    case STMT_TYPE_GLOBAL:
        list_safe_for_each(stmt->u.stmt_global, iter, next_iter) {
            stmt_global_t* global;

            list_erase(*iter);

            global = list_element(iter, stmt_global_t*, link);

            cstring_free(global->name);

            heap_free(global);
        }
        break;

    case STMT_TYPE_IF:
        expr_free(stmt->u.stmt_if.condition);
        stmt_free(stmt->u.stmt_if.if_block);
        list_safe_for_each(stmt->u.stmt_if.elifs, iter, next_iter) {
            stmt_t elif;

            list_erase(*iter);

            elif = list_element(iter, stmt_t, link);

            stmt_free(elif);
        }
        if (stmt->u.stmt_if.else_block)  {
            stmt_free(stmt->u.stmt_if.else_block);
        }
        break;

    case STMT_TYPE_ELIF:
        expr_free(stmt->u.stmt_elif.condition);
        stmt_free(stmt->u.stmt_elif.block);
        break;

    case STMT_TYPE_WHILE:
        expr_free(stmt->u.stmt_while.condition);
        stmt_free(stmt->u.stmt_while.block);
        break;

    case STMT_TYPE_FOR:
        if (stmt->u.stmt_for.init) {
            expr_free(stmt->u.stmt_for.init);
        }
        if (stmt->u.stmt_for.condition) {
            expr_free(stmt->u.stmt_for.condition);
        }
        if (stmt->u.stmt_for.post) {
            expr_free(stmt->u.stmt_for.post);
        }
        stmt_free(stmt->u.stmt_for.block);
        break;
    }

    heap_free(stmt);
}