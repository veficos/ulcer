

#include "stmt.h"
#include "alloc.h"

static stmt_t __stmt_new__(stmt_type_t type, long line, long column)
{
    stmt_t stmt = mem_alloc(sizeof(struct stmt_s));
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

            mem_free(global);
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

    mem_free(stmt);
}

parameter_t parameter_new(cstring_t name)
{
    parameter_t parameter = (parameter_t) mem_alloc(sizeof(struct parameter_s));
    if (!parameter) {
        return NULL;
    }

    parameter->name = name;

    return parameter;
}

void parameter_free(parameter_t parameter)
{
    cstring_free(parameter->name);
    mem_free(parameter);
}

function_t function_new_user(long line, long column, cstring_t name)
{
    function_t func = (function_t) mem_alloc(sizeof(struct function_s));
    if (!func) {
        return NULL;
    }

    func->name         = name;
    func->type         = FUNCTION_TYPE_USER;
    func->line         = line;
    func->column       = column;

    list_init(func->u.user.parameters);

    return func;
}

function_t function_new_native(cstring_t name, native_function_pt function)
{
    function_t func = (function_t) mem_alloc(sizeof(struct function_s));
    if (!func) {
        return NULL;
    }

    func->name              = name;
    func->type              = FUNCTION_TYPE_NATIVE;
    func->line              = 0;
    func->column            = 0;
    func->u.native.function = function;

    return func;
}

void function_free(function_t func)
{
    list_iter_t iter, next_iter;
    parameter_t parameter;

    cstring_free(func->name);

    switch (func->type) {
    case FUNCTION_TYPE_USER:
        list_safe_for_each(func->u.user.parameters, iter, next_iter) {
            list_erase(*iter);
            parameter = list_element(iter, parameter_t, link);
            parameter_free(parameter);
        }
        stmt_free(func->u.user.block);
        break;

    case FUNCTION_TYPE_NATIVE:
        break;
    }

    
    mem_free(func);
}
