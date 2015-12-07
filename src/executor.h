

#ifndef _ULCER_EXECUTOR_H_
#define _ULCER_EXECUTOR_H_

#include "config.h"
#include "environment.h"

typedef enum executor_result_e  executor_result_t;
typedef struct executor_s*      executor_t;

enum executor_result_e {
    EXECUTOR_RESULT_NORMAL,
    EXECUTOR_RESULT_RETURN,
    EXECUTOR_RESULT_BREAK,
    EXECUTOR_RESULT_CONTINUE,
};

executor_t executor_new(environment_t env);
void executor_free(executor_t exec);
void executor_run(executor_t exec);
executor_result_t executor_statement(environment_t env, statement_t stmt, bool toplevel);

#endif