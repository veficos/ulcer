

#ifndef _ULCER_EVALUATOR_H_
#define _ULCER_EVALUATOR_H_

#include "config.h"

void evaluator_expression(environment_t env, expression_t expr, bool toplevel);
const char* get_expression_type_string(expression_type_t type);
const char* get_value_type_string(value_type_t type);

#endif