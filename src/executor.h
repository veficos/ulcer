

#ifndef _ULCER_EXECUTOR_H_
#define _ULCER_EXECUTOR_H_

#include "config.h"
#include "environment.h"

typedef struct executor_s* executor_t;

executor_t executor_new(environment_t env);
void executor_free(executor_t exec);
void executor_run(executor_t exec);

#endif