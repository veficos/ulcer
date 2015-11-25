

#ifndef _ULCER_PARSER_H_
#define _ULCER_PARSER_H_

#include "config.h"
#include "module.h"
#include "lexer.h"
#include "list.h"

typedef struct parser_s* parser_t;

parser_t parser_new(lexer_t lex);
void     parser_free(parser_t parse);
module_t parser_generate_module(parser_t parse);

#endif
