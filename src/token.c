

#include "token.h"
#include "alloc.h"

#include <stdlib.h>

token_t token_new(const char *filename)
{
    token_t token = (token_t)heap_alloc(sizeof(struct token_s));
    if (!token) {
        return NULL;
    }
 
    token->filename = cstring_new(filename);
    token->token    = cstring_newempty(8);

    token->type     = TOKEN_TYPE_NIL;
    token->value    = TOKEN_VALUE_NIL;

    token->line     = 0;
    token->column   = 0;

    token->numberbase = 10;
    return token;
}

void token_free(token_t token)
{
    cstring_free(token->token);
    cstring_free(token->filename);
    heap_free(token);
}

void token_reset(token_t token, long line, long column, token_type_t type, token_value_t value)
{
    cstring_clear(token->token);

    token->type         = type;
    token->value        = value;
    token->line         = line;
    token->column       = column;
    token->numberbase   = 10;
}

token_t token_dup(token_t from)
{
    token_t to = (token_t)heap_alloc(sizeof(struct token_s));
    if (!to) {
        return NULL;
    }
  
    to->filename = cstring_dup(from->filename);
    to->token    = cstring_dup(from->token);

    to->type    = from->type;
    to->value   = from->value;

    to->line    = from->line;
    to->column  = from->column;

    to->numberbase = from->numberbase;
    return to;
}