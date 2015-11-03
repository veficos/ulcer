

#ifndef _ULCER_OBJECT_H_
#define _ULCER_OBJECT_H_

#include "config.h"
#include "executor.h"
#include "cstring.h"
#include "list.h"

typedef enum object_type_e object_type_t;
typedef struct object_s    object_t;
typedef struct heap_s*     heap_t;

enum object_type_e {
    OBJECT_TYPE_STRING,
};

struct object_s {
    object_type_t type;
    bool marked;

    union {
        cstring_t string;
    }u;

    list_node_t link;
};

struct heap_s {
    long current_heap_size;
    long current_heap_threshold;
    list_t objects;
};

#endif