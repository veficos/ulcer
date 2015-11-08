

#include "module.h"
#include "hashfn.h"
#include "alloc.h"

static int __module_function_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs);
static unsigned long __module_function_hashfn__(const hlist_node_t *hnode);

static hlist_node_ops_t function_hash_operators = {
    NULL,
    NULL,
    &__module_function_hashfn__,
    &__module_function_compare__,
    NULL,
};

module_t module_new(void)
{
    module_t module = (module_t) mem_alloc(sizeof(struct module_s));
    if (!module) {
        return NULL;
    }

    module->functions = hash_table_new(&function_hash_operators);

    list_init(module->statements);

    return module;
}

void module_free(module_t module)
{
    {
        /* statement */
        list_iter_t iter, next_iter;

        list_safe_for_each(module->statements, iter, next_iter) {
            list_erase(*iter);
            stmt_free(list_element(iter, stmt_t, link));
        }
    }
    
    {
        /* functions */
        hash_table_iter_t iter;
        function_t func;

        iter = hash_table_iter_new(module->functions);
        hash_table_for_each(module->functions, iter) {
            hlist_remove(iter.hn);
            func = hash_table_iter_element(iter, function_t, link);
            function_free(func);
        }
        hash_table_iter_free(iter);

        hash_table_free(module->functions);
    }

    mem_free(module);
}

bool module_add_function(module_t module, function_t func)
{
    return hash_table_insert(module->functions, &func->link);
}

void module_add_statment(module_t module, stmt_t stmt)
{
     list_push_back(module->statements, stmt->link);
}

function_t module_search_function(module_t module, cstring_t func_name)
{
    hlist_node_t *result;
    struct function_s search;

    search.name = func_name;

    result = hash_table_search(module->functions, &search.link);
    if (!result) {
        return NULL;
    }
    
    return hlist_element(result, function_t, link);
}

static int __module_function_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs)
{
    function_t lfun  = hlist_element(lhs, function_t, link);
    function_t rfunc = hlist_element(rhs, function_t, link);
    return cstring_cmp(lfun->name, rfunc->name);
}

static unsigned long __module_function_hashfn__(const hlist_node_t *hnode)
{
    function_t func = hlist_element(hnode, function_t, link);
    return (unsigned long) 
        murmur2_hash(func->name, cstring_length(func->name));
}