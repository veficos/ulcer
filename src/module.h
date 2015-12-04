

#ifndef _ULCER_MODULE_H_
#define _ULCER_MODULE_H_

#include "config.h"
#include "list.h"
#include "statement.h"

typedef struct module_s*         module_t;

struct module_s {
    list_t statements;
    list_t functions;
};

module_t module_new(void);
void     module_free(module_t module);
void     module_add_function(module_t module, statement_t function_stmt);
void     module_add_statment(module_t module, statement_t stmt);

#endif