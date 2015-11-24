

#ifndef _ULCER_CSTRING_H_
#define _ULCER_CSTRING_H_

#include "config.h"

typedef char* cstring_t;

cstring_t cstring_newlen(const void *data, unsigned long len);
cstring_t cstring_newempty(unsigned long len);
cstring_t cstring_new(const char *s);
void cstring_free(cstring_t cstr);
bool cstring_is_empty(cstring_t cstr);
void cstring_clear(cstring_t cstr);
unsigned long cstring_length(const cstring_t cstr);
unsigned long cstring_capacity(const cstring_t cstr);
unsigned long cstring_size(const cstring_t cstr);
void cstring_update_length(cstring_t cstr);
cstring_t cstring_dup(const cstring_t cstr);
cstring_t cstring_cat(cstring_t dst, const cstring_t src);
cstring_t cstring_catlen(cstring_t cstr, const void *s, unsigned long n);
cstring_t cstring_catstr(cstring_t cstr, const char *s);
cstring_t cstring_catch(cstring_t cstr, char ch);
cstring_t cstring_cpy(cstring_t dst, const cstring_t src);
cstring_t cstring_cpylen(cstring_t cstr, const void *s, unsigned long n);
cstring_t cstring_cpystr(cstring_t cstr, const char *s);
cstring_t cstring_cpych(cstring_t cstr, char ch);
int cstring_cmp(const cstring_t lhs, const cstring_t rhs);
void cstring_tolower(cstring_t cstr);
void cstring_toupper(cstring_t cstr);

#endif