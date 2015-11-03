


#include "alloc.h"
#include "executor.h"

static void __executor_statement__(executor_t inter, stmt_t stmt);
static void __executor_expr_statement__(executor_t inter, stmt_t stmt);

executor_t executor_new(list_t statements, functions_t functions)
{
    executor_t inter = (executor_t)mem_alloc(sizeof(struct executor_s));
    if (!inter) {
        return NULL;
    }

    inter->statements = statements;
    inter->functions  = functions;
    inter->stack      = array_new(128);

    return inter;
}

void executor_free(executor_t inter)
{
    array_free(inter->stack);
    mem_free(inter);
}

void executor_run(executor_t inter)
{
    list_iter_t iter;
    stmt_t stmt;

    list_for_each(inter->statements, iter) {
        stmt = list_element(iter, stmt_t, link);
        __executor_statement__(inter, stmt);
    }
}

static void __executor_statement__(executor_t inter, stmt_t stmt)
{
    switch (stmt->type)
    {
    case STMT_TYPE_EXPR:
        __executor_expr_statement__(inter, stmt);
        break;
    default:
        break;
    }
}

static void __executor_expr_statement__(executor_t inter, stmt_t stmt)
{

}