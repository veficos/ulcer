

#include "executor.h"
#include "environment.h"
#include "error.h"
#include "alloc.h"
#include "stmt.h"
#include "eval.h"

struct executor_s {
    environment_t env;
};

static executor_result_t __executor_statement__(environment_t env, stmt_t stmt);
static void __executor_expr_statement__(environment_t env, stmt_t stmt);
static executor_result_t __executor_if_statement__(environment_t env, stmt_t stmt);
static executor_result_t __executor_for_statement__(environment_t env, stmt_t stmt);

executor_t executor_new(environment_t env)
{
    executor_t exec = (executor_t) 
        mem_alloc(sizeof(struct executor_s));
    if (!exec) {
        return NULL;
    }

    exec->env = env;

    return exec;
}

void executor_free(executor_t exec)
{
    mem_free(exec);
}

void executor_run(executor_t exec)
{
    list_iter_t iter, next_iter;
    environment_t env;
    list_t statements;
    stmt_t stmt;

    env = exec->env;
    statements = env->module->statements;
    list_safe_for_each(statements, iter, next_iter) {
        stmt = list_element(iter, stmt_t, link);

        switch (__executor_statement__(env, stmt)) {
        case EXECUTOR_RESULT_BREAK:
            runtime_error(stmt->line, 
                          stmt->column,
                          "break outside loop");
            break;
        case EXECUTOR_RESULT_CONTINUE:
            runtime_error(stmt->line, 
                          stmt->column,
                          "continue outside loop");
            break;
        case EXECUTOR_RESULT_RETURN:
            runtime_error(stmt->line, 
                          stmt->column,
                          "return outside function");
            break;
        default:
            break;
        }
    }
}

static executor_result_t __executor_statement__(environment_t env, stmt_t stmt)
{
    switch (stmt->type)
    {
    case STMT_TYPE_EXPR:
        __executor_expr_statement__(env, stmt);
        array_pop(env->stack);
        return EXECUTOR_RESULT_NORMAL;

    case STMT_TYPE_BREAK:
        return EXECUTOR_RESULT_BREAK;

    case STMT_TYPE_CONTINUE:
        return EXECUTOR_RESULT_CONTINUE;

    case STMT_TYPE_RETURN:
        eval_expression(env, stmt->u.expr);
        return EXECUTOR_RESULT_RETURN;

    case STMT_TYPE_BLOCK:
        return executor_block_statement(env, stmt);

    case STMT_TYPE_GLOBAL:
    case STMT_TYPE_IF:
        return __executor_if_statement__(env, stmt);

    case STMT_TYPE_ELIF:
    case STMT_TYPE_WHILE:
    case STMT_TYPE_FOR:
        return __executor_for_statement__(env, stmt);
    }

    return EXECUTOR_RESULT_NORMAL;
}

static void __executor_expr_statement__(environment_t env, stmt_t stmt)
{
    eval_expression(env, stmt->u.expr);
}

static executor_result_t __executor_if_statement__(environment_t env, stmt_t stmt)
{
    value_t value;
    stmt_if_t stmt_if;
    executor_result_t result = EXECUTOR_RESULT_NORMAL;

    stmt_if = stmt->u.stmt_if;

    eval_expression(env, stmt_if.condition);
    value = array_index(env->stack, array_length(env->stack) - 1);
    array_pop(env->stack);

    if (value->type != VALUE_TYPE_BOOL) {
        runtime_error(stmt->line, 
                      stmt->column,
                      "%s cannot be converted to bool",
                      value_type_string(value));
    }

    if (value->u.bool_value) {
        result = executor_block_statement(env, stmt_if.if_block);
    }

    return result;
}

static executor_result_t __executor_for_statement__(environment_t env, stmt_t stmt)
{
    value_t value;
    stmt_for_t stmt_for;
    executor_result_t result;

    stmt_for = stmt->u.stmt_for;

    if (stmt_for.init) {
        eval_expression(env, stmt_for.init);
        array_pop(env->stack);
    }

    while (true) {
        if (stmt_for.condition) {
            eval_expression(env, stmt_for.condition);
            value = array_index(env->stack, array_length(env->stack) - 1);
            array_pop(env->stack);

            if (value->type != VALUE_TYPE_BOOL) {
                runtime_error(stmt->line, 
                              stmt->column,
                              "%s cannot be converted to bool",
                              value_type_string(value));
            }

            if (!value->u.bool_value) {
                break;
            }
        }

        result = __executor_statement__(env, stmt_for.block);

        if (result == EXECUTOR_RESULT_RETURN) {
            return EXECUTOR_RESULT_RETURN;
        } else if (result == EXECUTOR_RESULT_BREAK) {
            return EXECUTOR_RESULT_NORMAL;
        }

        if (stmt_for.post) {
            eval_expression(env, stmt_for.post);
            array_pop(env->stack);
        }
    }

    return EXECUTOR_RESULT_NORMAL;
}

executor_result_t executor_block_statement(environment_t env, stmt_t stmt_block)
{
    list_iter_t iter, next_iter;
    executor_result_t result;
    list_t statements;
    stmt_t stmt;

    statements = stmt_block->u.block;
    list_safe_for_each(statements, iter, next_iter) {
        stmt = list_element(iter, stmt_t, link);
        result = __executor_statement__(env, stmt);

        if (result != EXECUTOR_RESULT_NORMAL) {
            return result;
        }
    }

    return EXECUTOR_RESULT_NORMAL;
}