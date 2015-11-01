

#ifndef _ULCER_EVAL_H_
#define _ULCER_EVAL_H_

#include "config.h"
#include "interpreter.h"
#include "ulcer.h"
#include "expr.h"

value_t eval_expression(interpreter_t inter, expr_t expr);

#endif