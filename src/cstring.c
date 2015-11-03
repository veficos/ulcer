

#include "cstring.h"
#include "alloc.h"

#include <string.h>
#include <ctype.h>

struct cstring_hdr_s {
    unsigned int length;
    unsigned int free;
    char buffer[1];
};

#define cstring_of(cstr)                                                      \
    ((struct cstring_hdr_s *)(((unsigned char*)(cstr)) -                      \
        (unsigned char*)(&(((struct cstring_hdr_s *)0)->buffer))))

#ifndef CSTRING_MAX_PREALLOC
#define CSTRING_MAX_PREALLOC (1024*1024)
#endif

static cstring_t __cstring_make_some_space__(cstring_t cstr, unsigned long addlen);

cstring_t cstring_newlen(const void *data, unsigned long len)
{
    struct cstring_hdr_s *hdr;

    if (data) {
        hdr = mem_alloc(sizeof(struct cstring_hdr_s) + len);
    } else {
        hdr = mem_calloc(sizeof(struct cstring_hdr_s) + len);
    }

    if (!hdr) {
        return NULL;
    }

    hdr->length = len;
    hdr->free = 0;

    if (data && len) {
        memcpy(hdr->buffer, data, len);
    }

    hdr->buffer[len] = '\0';

    return (cstring_t) hdr->buffer;
}

cstring_t cstring_newempty(unsigned long len)
{
    struct cstring_hdr_s *hdr;

    hdr = mem_alloc(sizeof(struct cstring_hdr_s) + len);
    if (!hdr) {
        return NULL;
    }

    hdr->length = 0;
    hdr->free = len;
    hdr->buffer[len] = '\0';

    return hdr->buffer;
}

cstring_t cstring_new(const char *s)
{
    unsigned long len = s == NULL ? 0 : strlen(s);
    return cstring_newlen(s, len);
}

void cstring_free(cstring_t cstr)
{
    if (cstr) {
        mem_free(cstring_of(cstr));
    }
}

bool cstring_is_empty(cstring_t cstr)
{
    return cstring_length(cstr) == 0;
}

unsigned long cstring_length(const cstring_t cstr)
{
    return cstring_of(cstr)->length;
}

unsigned long cstring_capacity(const cstring_t cstr)
{
    return cstring_of(cstr)->free;
}

unsigned long cstring_size(const cstring_t cstr)
{
    struct cstring_hdr_s *hdr = cstring_of(cstr);
    return sizeof(struct cstring_hdr_s) + hdr->length + hdr->free;
}

void cstring_update_length(cstring_t cstr)
{
    struct cstring_hdr_s *hdr = cstring_of(cstr);
    unsigned long reallen = strlen(cstr);
    hdr->free += (hdr->length - reallen);
    hdr->length = reallen;
}

void cstring_clear(cstring_t cstr)
{
    struct cstring_hdr_s *hdr = cstring_of(cstr);
    hdr->free += hdr->length;
    hdr->length = 0;
    hdr->buffer[0] = '\0';
}

cstring_t cstring_dup(const cstring_t cstr)
{
    return cstring_newlen(cstr, cstring_length(cstr)); 
}

cstring_t cstring_cat(cstring_t dst, const cstring_t src) 
{
    return cstring_catlen(dst, src, cstring_length(src));
}

cstring_t cstring_catlen(cstring_t cstr, const void *s, unsigned long n)
{
    struct cstring_hdr_s *hdr = cstring_of(cstr);

    cstr = __cstring_make_some_space__(cstr, n);
    if (!cstr) {
        return NULL;
    }
    
    hdr = cstring_of(cstr);

    memcpy(&hdr->buffer[hdr->length], s, n);
    hdr->length += n;
    hdr->free -= n;
    hdr->buffer[hdr->length] = '\0';
    return cstr;
}

cstring_t cstring_catstr(cstring_t cstr, const char *s)
{
    return cstring_catlen(cstr, s, strlen(s));
}

cstring_t cstring_catch(cstring_t cstr, char ch)
{
    return cstring_catlen(cstr, &ch, sizeof(char));
}

cstring_t cstring_cpy(cstring_t dst, const cstring_t src)
{
    return cstring_cpylen(dst, src, cstring_length(src));
}

cstring_t cstring_cpylen(cstring_t cstr, const void *s, unsigned long n)
{
    struct cstring_hdr_s *hdr = cstring_of(cstr);
    unsigned long total = hdr->free + hdr->length;
    
    if (total < n) {
        cstr = __cstring_make_some_space__(cstr, n - hdr->length);
        if (!cstr) {
            return NULL;
        }
        hdr = cstring_of(cstr);
        total = hdr->free + hdr->length;
    }

    memcpy(cstr, s, n);
    cstr[n] = '\0';
    hdr->length = n;
    hdr->free = total - n;

    return cstr;
}

cstring_t cstring_cpystr(cstring_t cstr, const char *s)
{
    return cstring_cpylen(cstr, s, strlen(s));
}

cstring_t cstring_cpych(cstring_t cstr, char ch)
{
    return cstring_cpylen(cstr, &ch, sizeof(char));
}

cstring_t cstring_reserve(cstring_t cstr, unsigned long len)
{
    return __cstring_make_some_space__(cstr, len);
}

cstring_t cstring_resize(cstring_t cstr, unsigned long n)
{
    struct cstring_hdr_s *hdr = cstring_of(cstr);
    unsigned long total = hdr->length + hdr->free;

    if (total > n) {
        hdr->free = total - n;
        hdr->length = n;
        hdr->buffer[0] = '\0';
        return cstr;
    }

    cstr = __cstring_make_some_space__(cstr, n - hdr->length);
    if (!cstr) {
        return NULL;
    }

    hdr = cstring_of(cstr);

    memset(&hdr->buffer[hdr->length], 0, n - hdr->length);

    hdr->length = n;
    hdr->free = 0;

    return cstr;
}

cstring_t cstring_shrink_to_fit(cstring_t cstr) 
{
    struct cstring_hdr_s *hdr = cstring_of(cstr);

    hdr = mem_realloc(hdr, sizeof(struct cstring_hdr_s) + hdr->length);

    hdr->free = 0;
    
    return hdr->buffer;
}

int cstring_cmp(const cstring_t lhs, const cstring_t rhs)
{
    unsigned long lhslen, rhslen, minlen;
    int cmp;

    lhslen = cstring_length(lhs);
    rhslen = cstring_length(rhs);
    minlen = lhslen < rhslen ? lhslen : rhslen;
    
    cmp = memcmp(lhs, rhs, minlen);

    if (cmp == 0) {
        return lhslen - rhslen;
    }

    return cmp;
}

void cstring_tolower(cstring_t cstr)
{
    unsigned long i, len = cstring_length(cstr);
    for (i = 0; i < len; i++) {
        cstr[i] = (char) tolower(cstr[i]);
    }
}

void cstring_toupper(cstring_t cstr)
{
    unsigned long i, len = cstring_length(cstr);
    for (i = 0; i < len; i++) {
        cstr[i] = (char) toupper(cstr[i]);
    }
}

static cstring_t __cstring_make_some_space__(cstring_t cstr, unsigned long addlen)
{
    struct cstring_hdr_s *hdr, *newhdr;
    unsigned long newlen;

    hdr = cstring_of(cstr);
    if (hdr->free > addlen) {
        return cstr;
    }

    newlen = hdr->length + addlen;
    if (newlen <= CSTRING_MAX_PREALLOC) {
        newlen *= 2;
    } else {
        newlen += CSTRING_MAX_PREALLOC;
    }

    newhdr = (struct cstring_hdr_s *)
        mem_realloc(hdr, sizeof(struct cstring_hdr_s) + newlen);
    if (!newhdr) {
        return NULL;
    }

    newhdr->free = newlen - newhdr->length;
    return (cstring_t) newhdr->buffer;
}