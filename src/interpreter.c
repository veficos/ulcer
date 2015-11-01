

#include "interpreter.h"
#include "alloc.h"

static void __interpreter_execute_statement__(interpreter_t inter, stmt_t stmt);
static void __interpreter_execute_expr_statement__(interpreter_t inter, stmt_t stmt);

interpreter_t interpreter_new(list_t statements, functions_t functions)
{
    interpreter_t inter = (interpreter_t)heap_alloc(sizeof(struct interpreter_s));
    if (!inter) {
        return NULL;
    }

    inter->statements = statements;
    inter->functions  = functions;

    return inter;
}

void interpreter_free(interpreter_t inter)
{
    heap_free(inter);
}

void interpreter_execute(interpreter_t inter)
{
    list_iter_t iter;
    stmt_t stmt;

    list_for_each(inter->statements, iter) {
        stmt = list_element(iter, stmt_t, link);
        __interpreter_execute_statement__(inter, stmt);
    }
}

static void __interpreter_execute_statement__(interpreter_t inter, stmt_t stmt)
{
    switch (stmt->type)
    {
    case STMT_TYPE_EXPR:
        __interpreter_execute_expr_statement__(inter, stmt);
        break;
    default:
        break;
    }
}

static void __interpreter_execute_expr_statement__(interpreter_t inter, stmt_t stmt)
{

}