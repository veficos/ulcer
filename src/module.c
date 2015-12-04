

#include "module.h"
#include "hashfn.h"
#include "alloc.h"

module_t module_new(void)
{
    module_t module = (module_t) mem_alloc(sizeof(struct module_s));
    if (!module) {
        return NULL;
    }

    list_init(module->functions);
    list_init(module->statements);

    return module;
}

void module_free(module_t module)
{
    list_iter_t iter, next_iter;

    list_safe_for_each(module->statements, iter, next_iter) {
        list_erase(module->statements, *iter);
        statement_free(list_element(iter, statement_t, llink));
    }

    list_safe_for_each(module->functions, iter, next_iter) {
        list_erase(module->statements, *iter);
        statement_free(list_element(iter, statement_t, llink));
    }

    mem_free(module);
}

void module_add_function(module_t module, statement_t function_stmt)
{
    list_push_back(module->functions, function_stmt->llink);
}

void module_add_statment(module_t module, statement_t stmt)
{
    list_push_back(module->statements,  stmt->llink);
}