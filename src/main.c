
#include "source_code.h"
#include "parser.h"
#include "lexer.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** args)
{
    source_code_t sc;
    module_t module;
    parser_t parse;
    lexer_t lex;
  
    if (argc < 2) {
        printf("usage: ulcer souce_code.u\n");
        exit(-1);
    }
    
    sc = source_code_new(args[1], SOURCE_CODE_TYPE_FILE);
    lex = lexer_new(sc);
    parse = parser_new(lex);

    module = parser_generate_module(parse);

    parser_free(parse);
    lexer_free(lex);
    source_code_free(sc);

    return 0;
}