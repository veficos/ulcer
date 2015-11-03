

#ifndef _ULCER_EXECUTOR_H_
#define _ULCER_EXECUTOR_H_

#include "config.h"
#include "list.h"
#include "array.h"
#include "object.h"
#include "function.h"

typedef struct executor_s* executor_t;

struct executor_s {
    list_t      statements;
    functions_t functions;
    array_t     stack;
    heap_t      heap;
};

executor_t executor_new(list_t statements, functions_t functions);
void executor_free(executor_t inter);
void executor_run(executor_t inter);

#endif