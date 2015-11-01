

#ifndef _ULCER_PARSER_H_
#define _ULCER_PARSER_H_

#include "config.h"
#include "lexer.h"
#include "list.h"
#include "function.h"

typedef struct parser_s* parser_t;

parser_t parser_new(lexer_t lex);
void parser_free(parser_t parse);
void parser_translation(parser_t parse);
list_t parser_get_statements(parser_t parse);
functions_t parser_get_functions(parser_t parse);

#endif
