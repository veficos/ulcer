

#ifndef _ULCER_NATIVE_H_
#define _ULCER_NATIVE_H_

#include "config.h"
#include "environment.h"

void setup_native_module(environment_t env);
void* native_check_pointer_value(value_t value);
int native_check_int_value(value_t value);
const char* native_check_string_value(value_t value);
bool native_check_bool_value(value_t value);
double native_check_double_value(value_t value);

#endif
