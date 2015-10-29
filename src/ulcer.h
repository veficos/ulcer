

#ifndef _ULCER_H_
#define _ULCER_H_

#include "cstring.h"

typedef enum value_type_e {
    VALUE_TYPE_CHAR,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_LONG,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_STRING,
    VALUE_TYPE_NULL,
}value_type_t;

typedef struct value_s {
    value_type_t type;
    union {
        char      ch;
        bool      b;
        int       i;
        long      l;
        float     f;
        double    d;
        cstring_t s;
    }u;
}value_t;

#endif