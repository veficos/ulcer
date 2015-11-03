

#ifndef _ULCER_ARRAY_H_
#define _ULCER_ARRAY_H_

#include "config.h"

typedef struct array_s {
    void *elts;
    unsigned long nelts;
    unsigned long size;
    unsigned long nalloc;
}* array_t;

typedef int (*array_compare_pt)(const void *, const void *);
typedef bool (*array_magic_pt)(void *);

#define array_is_empty(a)                                                     \
    (a)->nelts == 0

#define array_base(a, type)                                                   \
    ((type)((a)->elts))

#define array_clear(a)                                                        \
    ((a)->nelts = 0) 

#define array_length(a)                                                       \
    (a)->nelts

#define array_capacity(a)                                                     \
    ((a)->nalloc - (a)->nelts)

#define array_pop(a)                                                          \
    do {                                                                      \
        if ((a)->nelts != 0) {                                                \
            (a)->nelts--;                                                     \
        }                                                                     \
    } while (false)

#define array_pop_n(a, n)                                                     \
    do {                                                                      \
        if (n > (a)->nelts) {                                                 \
            (a)->nelts = 0;                                                   \
        } else {                                                              \
            (a)->nelts -= n;                                                  \
        }                                                                     \
    } while (false)

#define array_for_each(a, base, index)                                        \
    for ((base) = (a)->elts, (index) = 0;                                     \
         (unsigned long)(index) < (a)->nelts;                                 \
         (index)++)

array_t array_new(unsigned long size);
array_t array_newlen(unsigned long size, unsigned long n);
void array_free(array_t a);
array_t array_dup(array_t a);
void array_swap(array_t lhs, array_t rhs);
void array_shrink_to_fit(array_t a);
bool array_reserve(array_t a, unsigned long n);
bool array_resize(array_t a, unsigned long n);
void *array_push(array_t a);
void *array_push_n(array_t a, unsigned long n);
bool array_erase(array_t a, unsigned int index);
bool array_erase_n(array_t a, unsigned int index, unsigned long n);
bool array_insert(array_t a, unsigned int index, void *ptr);
bool array_insert_n(array_t a, unsigned int index, void *ptr, unsigned long n);
void array_range(array_t a, unsigned long start, unsigned long end);
void array_magic(array_t a, array_magic_pt magic);

#endif
