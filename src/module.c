

#include "module.h"
#include "hashfn.h"
#include "alloc.h"

module_t module_new(void)
{
    module_t module;
    statements_t statements;

    module = (module_t) mem_alloc(sizeof(struct module_s));
    if (!module) {
        return NULL;
    }

    statements = (statements_t) mem_alloc(sizeof(struct statement_s));
    if (!statements) {
        return NULL;
    }

    module->statements = statements;

    list_init(module->functions);
    list_init(module->statements->stmts);

    return module;
}

void module_free(module_t module)
{
    list_iter_t iter, next_iter;

    list_safe_for_each(module->statements->stmts, iter, next_iter) {
        list_erase(module->statements->stmts, *iter);
        statement_free(list_element(iter, statement_t, link));
    }

    list_safe_for_each(module->functions, iter, next_iter) {
        list_erase(module->functions, *iter);
        statement_free(list_element(iter, statement_t, link));
    }

    mem_free(module->statements);
    mem_free(module);
}

void module_add_function(module_t module, statement_t function_stmt)
{
    list_push_back(module->functions, function_stmt->link);
}

void module_add_statment(module_t module, statement_t stmt)
{
    list_push_back(module->statements->stmts,  stmt->link);
}