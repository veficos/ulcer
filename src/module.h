

#ifndef _ULCER_MODULE_H_
#define _ULCER_MODULE_H_

#include "config.h"
#include "list.h"
#include "stmt.h"

typedef struct module_s* module_t;

struct module_s {
    list_t statements;
    hash_table_t functions;
};

module_t module_new(void);
void module_free(module_t module);
bool module_add_function(module_t module, function_t func);
void module_add_statment(module_t module, stmt_t stmt);

#endif