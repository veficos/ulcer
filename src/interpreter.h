

#ifndef _ULCER_INTERPRETER_H_
#define _ULCER_INTERPRETER_H_

#include "config.h"
#include "list.h"
#include "object.h"
#include "function.h"

typedef struct interpreter_s* interpreter_t;

struct interpreter_s {
    list_t      statements;
    functions_t functions;
    heap_t      heap;
};

interpreter_t interpreter_new(list_t statements, functions_t functions);
void interpreter_free(interpreter_t inter);
void interpreter_execute(interpreter_t inter);

#endif