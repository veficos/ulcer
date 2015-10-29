

#ifndef _NCC_LEXER_H_
#define _NCC_LEXER_H_

#include "config.h"
#include "token.h"
#include "source_code.h"

typedef struct lexer_s* lexer_t;

lexer_t lexer_new(source_code_t source_code);
void lexer_free(lexer_t lex);
token_t lexer_peek(lexer_t lex);
token_t lexer_next(lexer_t lex);
void lexer_unget(lexer_t lex, token_t tok);

#endif
