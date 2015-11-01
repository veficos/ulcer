

#include "function.h"
#include "hashfn.h"
#include "hlist.h"
#include "alloc.h"

parameter_t parameter_new(cstring_t name)
{
    parameter_t parameter = (parameter_t) heap_alloc(sizeof(struct parameter_s));
    if (!parameter) {
        return NULL;
    }

    parameter->name = name;

    return parameter;
}

void parameter_free(parameter_t parameter)
{
    cstring_free(parameter->name);
    heap_free(parameter);
}

function_t function_new(long line, long column, cstring_t name)
{
    function_t function = (function_t) heap_alloc(sizeof(struct function_s));
    if (!function) {
        return NULL;
    }

    function->name         = name;
    function->line         = line;
    function->column       = column;
   
    list_init(function->u.self.parameters);

    return function;
}

void function_free(function_t function)
{
    cstring_free(function->name);
    stmt_free(function->u.self.block);
    heap_free(function);
}

static int __function_compare__(const hlist_node_t *lhs, const hlist_node_t *rhs)
{
    function_t lfun  = hlist_element(lhs, function_t, link);
    function_t rfunc = hlist_element(rhs, function_t, link);
    return cstring_cmp(lfun->name, rfunc->name);
}

unsigned long __function_hashfn__(const hlist_node_t *hnode)
{
    function_t function = hlist_element(hnode, function_t, link);
    return (unsigned long) 
        murmur2_hash(function->name, cstring_length(function->name));
}

functions_t function_library_new()
{
    functions_t fl = (functions_t) 
        heap_alloc(sizeof(struct functions_s));
    if (!fl) {
        return NULL;
    }

    fl->ops.construct   = NULL;
    fl->ops.destructor  = NULL;
    fl->ops.dup         = NULL;
    fl->ops.compare     = &__function_compare__;
    fl->ops.hashfn      = &__function_hashfn__;
    fl->htable          = hash_table_new(&fl->ops);

    return fl;
}

bool functions_add(functions_t fl, function_t function)
{
    return hash_table_insert(fl->htable, &function->link);
}

bool functions_del(functions_t fl, const char* function_name)
{
    struct function_s function;
    hlist_node_t* found;

    function.name = cstring_new(function_name);

    found = hash_table_search(fl->htable, &function.link);
    if (!found) {
        return false;
    }

    cstring_free(function.name);

    function_free(hlist_element(found, function_t, link));

    return true;
}

void functions_free(functions_t fl)
{
    hash_table_iter_t iter;
    function_t function;

    iter = hash_table_iter_new(fl->htable);
    hash_table_for_each(fl->htable, iter) {
        hlist_remove(iter.hn);
        function = hash_table_iter_element(iter, function_t, link);
        function_free(function);
    }
    hash_table_iter_free(iter);
    
    hash_table_free(fl->htable);
    heap_free(fl);
}