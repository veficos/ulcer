

#ifndef _ULCER_HEAP_H_
#define _ULCER_HEAP_H_

#include "config.h"
#include "executor.h"
#include "cstring.h"
#include "list.h"

object_t heap_alloc_object(executor_t exec, object_type_t type);
object_t heap_alloc_string(executor_t exec);

#endif