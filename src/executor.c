

#include "executor.h"
#include "environment.h"
#include "statement.h"
#include "evaluator.h"
#include "error.h"
#include "alloc.h"

#include <assert.h>

struct executor_s {
    environment_t env;
};

static executor_result_t __executor_if_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_elif_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_for_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_while_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_block_statement__(environment_t env, statement_t stmt_block, bool toplevel);

executor_t executor_new(environment_t env)
{
    executor_t exec = (executor_t) mem_alloc(sizeof(struct executor_s));
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
    list_iter_t   iter;
    environment_t env;
    statements_t  stmts;
    statement_t   stmt;

    env = exec->env;

    assert(!stack_is_empty(env->statement_stack));

    stmts = stack_element(stack_pop(env->statement_stack), statements_t, link);

    list_for_each(stmts->stmts, iter) {
        stmt = list_element(iter, statement_t, link);
        switch (executor_statement(env, stmt, true)) {
        case EXECUTOR_RESULT_BREAK:
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "break outside loop");
            break;
        case EXECUTOR_RESULT_CONTINUE:
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "continue outside loop");
            break;
        case EXECUTOR_RESULT_RETURN:
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "return outside function");
            break;
        default:
            break;
        }
    }
}

executor_result_t executor_statement(environment_t env, statement_t stmt, bool toplevel)
{
    switch (stmt->type) {
    case STATEMENT_TYPE_EXPRESSION:
        evaluator_expression(env, stmt->u.expr, toplevel);
        environment_clear_stack(env);
        return EXECUTOR_RESULT_NORMAL;

    case STATEMENT_TYPE_CONTINUE:
        return EXECUTOR_RESULT_CONTINUE;

    case STATEMENT_TYPE_BREAK:
        return EXECUTOR_RESULT_BREAK;

    case STATEMENT_TYPE_RETURN:
        return EXECUTOR_RESULT_RETURN;

    default:
        return EXECUTOR_RESULT_NORMAL;
    }

    return EXECUTOR_RESULT_NORMAL;
}