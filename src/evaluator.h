

#ifndef _ULCER_EVALUATOR_H_
#define _ULCER_EVALUATOR_H_

#include "config.h"

void evaluator_expression(environment_t env, expression_t expr);
void evaluator_binary_value(environment_t env, long line, long column, expression_type_t type, value_t left, value_t right);
const char* get_expression_type_string(expression_type_t type);
const char* get_value_type_string(value_type_t type);

#endif