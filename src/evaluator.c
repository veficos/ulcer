

#include "executor.h"
#include "environment.h"
#include "statement.h"
#include "evaluator.h"
#include "error.h"
#include "alloc.h"

struct executor_s {
    environment_t env;
};

static executor_result_t __executor_if_statement__(environment_t env, statement_t stmt);
static executor_result_t __executor_elif_statement__(environment_t env, statement_t stmt);
static executor_result_t __executor_for_statement__(environment_t env, statement_t stmt);
static executor_result_t __executor_while_statement__(environment_t env, statement_t stmt);
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
    list_t        statements;
    statement_t   stmt;

    env        = exec->env;
    
}