

#include "array.h"
#include "alloc.h"

#include <string.h>

static bool __array_resize__(array_t a, unsigned long n);

array_t array_create(unsigned long size)
{
    array_t a;

    a = (array_t) heap_alloc(sizeof(struct array_s));
    if (!a) {
        return NULL;
    }
 
    a->elts   = NULL;
    a->nelts  = 0;
    a->nalloc = 0;
    a->size   = size;

    return a;
}

array_t array_create_n(unsigned long size, unsigned long n)
{
    array_t a;

    a = (array_t) heap_alloc(sizeof(struct array_s));
    if (!a) {
        return NULL;
    }

    a->elts = (array_t) heap_alloc(size * n);
    if (!a->elts) {
        return NULL;
    }

    a->nelts  = 0;
    a->nalloc = n;
    a->size   = size;

    return a;
}

void array_destroy(array_t a)
{
    if (a->elts) {
        heap_free(a->elts);
    }
    heap_free(a);
}

array_t array_dup(array_t a)
{
    array_t dst = array_create_n(a->size, a->nelts);
    if (!dst) {
        return NULL;
    }

    memcpy(array_push_n(dst, a->nelts), a->elts, a->nelts * a->size);

    return dst;
}

void array_swap(array_t lhs, array_t rhs)
{
    struct array_s tmp;
    tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

void array_shrink_to_fit(array_t a)
{
    __array_resize__(a, a->nelts);
}

bool array_reserve(array_t a, unsigned long n)
{
    unsigned long nfree = a->nalloc - a->nelts;
    if (nfree >= n) {
        return true;
    }

    if (!__array_resize__(a, a->nelts + (n - nfree))) {
        return false;
    }

    return true;
}

bool array_resize(array_t a, unsigned long n)
{
    void *elt;
    unsigned long  nalloc;

    if (a->nelts >= n) {
        a->nelts = n;
        return true;
    }
    
    nalloc = n - a->nelts;
    elt    = array_push_n(a, nalloc);
    if (!elt) {
        return false;
    }

    memset(elt, 0, nalloc * a->size);
    return true;
}

void *array_push(array_t a)
{
    void *elt;

    if (a->nelts == a->nalloc) {
        unsigned long n;

        if (a->nalloc == 0) {
            n = 2;

        } else {
            n = a->nalloc * 2;
        }

        if (!__array_resize__(a, n)) {
            return NULL;
        }
    }

    elt = (unsigned char *)a->elts + a->size * a->nelts;
    a->nelts++;

    return elt;
}

void *array_push_n(array_t a, unsigned long n) 
{
    void *elt;

    if (a->nelts + n > a->nalloc) {
        unsigned long nalloc = 2 * (n >= a->nalloc ? n : a->nalloc);
        if (!__array_resize__(a, nalloc)) {
            return NULL;
        }
    }
    
    elt = (unsigned char *)a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}

bool array_erase(array_t array, unsigned int index)
{
    unsigned char *p;
    unsigned int  move_size;

    if (index >= array->nelts) {
        return false;
    }

    p = (unsigned char *)array->elts;
    move_size = array->size * (array->nelts - (index + 1));

    if (move_size) {
        memcpy(p + array->size * index,
            p + array->size * (index + 1), move_size);
    }

    array->nelts--;
    return true;
}

bool array_erase_n(array_t a, unsigned int index, unsigned long n)
{
    unsigned char *p;
    unsigned int  move_size;

    if ((index >= a->nelts) || (index + n > a->nelts)) {
        return false;
    }

    p = (unsigned char *)a->elts;
    move_size = a->size * (a->nelts - index - n);

    if (move_size && n) {
        memcpy(p + a->size * index,
            p + a->size * (index + n), move_size);
    }

    a->nelts -= n;
    return true;
}

void array_range(array_t a, unsigned long start, unsigned long end)
{
    unsigned long newlen;

    if (a->nelts == 0) {
        return ;
    }

    start = start >= 0 ? start : 0;
    end = end >= 0 ? end : a->nelts - 1;
    newlen = (unsigned long)(start > end ? 0 : end - start + 1);
    newlen = a->nelts > newlen ? newlen : a->nelts;

    if (start && newlen) {
        memmove(a->elts,
            (unsigned char*)a->elts + start * a->size,
            newlen * a->size);
    }

    a->nelts = newlen;
}

void array_magic(array_t a, array_magic_pt magic)
{
    unsigned char *base;
    int index;

    array_for_each(a, base, index) {
        if (!magic((void*)(base + (index * a->size)))) {
            return ;
        }
    }
}

bool array_insert(array_t a, unsigned int index, void *ptr)
{
    unsigned char *p;
    unsigned int  move_size;

    if (index > a->nelts) {
        return false;

    } else if (a->nalloc == a->nelts) {
        if (!array_push(a)) {
            return false;
        }

    } else {
        a->nelts++;
    }

    p = (unsigned char *)a->elts;
    move_size = a->size * (a->nelts - index);

    if (move_size) {
        memmove(p + a->size * (index + 1), p + a->size * index, move_size);
    }

    memcpy(p + a->size * index, ptr, a->size);

    return true;
}

bool array_insert_n(array_t a, unsigned int index, void *ptr, unsigned long n)
{

    unsigned char *p;
    unsigned int  move_size;

    if (index > a->nelts) {
        return false;

    } else if (a->nalloc + n > a->nelts) {
        if (!array_push_n(a, n)) {
            return false;
        }

    } else {
        a->nelts += n;
    }

    p = (unsigned char *)a->elts;
    move_size = a->size * (a->nelts - (index + n));

    if (move_size) {
        memmove(p + a->size * (index + n), p + a->size * index, move_size);
    }

    memcpy(p + a->size * index, ptr, a->size * n);

    return true;
}

static bool __array_resize__(array_t a, unsigned long n)
{
    unsigned long size = a->size * n;
 
    if (size) {
        a->elts = heap_realloc(a->elts, size);
        if (!a->elts) {
            return false;
        }
    }

    a->nalloc = n;
    
    return true;
}
