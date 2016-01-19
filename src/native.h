

#ifndef _ULCER_NATIVE_H_
#define _ULCER_NATIVE_H_

#include "config.h"
#include "environment.h"

void import_native_library(environment_t env);
void* native_check_pointer_value(value_t value);
int native_check_int_value(value_t value);

#endif