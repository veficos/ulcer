

#include "executor.h"
#include "environment.h"
#include "alloc.h"
#include "stmt.h"
#include "eval.h"

struct executor_s {
    environment_t env;
};

static void __executor_statement__(environment_t env, stmt_t stmt);
static void __executor_expr_statement__(environment_t env, stmt_t stmt);

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
    environment_t env = exec->env;
    list_t statements;
    stmt_t stmt;

    statements = env->module->statements;
    list_safe_for_each(statements, iter, next_iter) {
        stmt = list_element(iter, stmt_t, link);
        __executor_statement__(env, stmt);
    }
}

static void __executor_statement__(environment_t env, stmt_t stmt)
{
    switch (stmt->type)
    {
    case STMT_TYPE_EXPR:
        __executor_expr_statement__(env, stmt);
        break;

    case STMT_TYPE_BREAK:
    case STMT_TYPE_CONTINUE:
    case STMT_TYPE_RETURN:
    case STMT_TYPE_BLOCK:
    case STMT_TYPE_GLOBAL:
    case STMT_TYPE_IF:
    case STMT_TYPE_ELIF:
    case STMT_TYPE_WHILE:
    case STMT_TYPE_FOR:
        break;
    default:
        break;
    }
}

static void __executor_expr_statement__(environment_t env, stmt_t stmt)
{
    eval_expression(env, stmt->u.expr);
}