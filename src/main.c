
#include "config.h"
#include "alloc.h"
#include "lexer.h"
#include "token.h"
#include "stack.h"
#include "list.h"
#include "parser.h"
#include "executor.h"
#include "source_code.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

static  void __dump_lexer_result__(lexer_t lex)
{
    const char *types[] = {
        "nil        ",
        "integer    ",
        "float      ",
        "boolean    ",
        "literal    ",
        "indentifier",
        "keyword    ",
        "operator   ",
        "delimiter  ",
        "end        "
    };

    const char *values[] = {
        "nil            ",
        "lp             ",
        "rp             ",
        "lc             ",
        "rc             ",
        "lb             ",
        "rb             ",
        "colon          ",
        "semicolon      ",
        "comma          ",
        "and            ",
        "or             ",
        "assign         ",
        "eq             ",
        "neq            ",
        "lt             ",
        "leq            ",
        "gt             ",
        "geq            ",
        "add            ",
        "sub            ",
        "mul            ",
        "div            ",
        "mod            ",
        "add assign     ",
        "sub assign     ",
        "mul assign     ",
        "div assign     ",
        "mod assign     ",
        "inc            ",
        "dec            ",
        "not            ",
        "dot            ",
        "literal int    ",
        "literal uint   ",
        "literal long   ",
        "literal ulong  ",
        "literal float  ",
        "literal double ",
        "literal char   ",
        "literal string ",

        "function       ",
        "if             ",
        "else           ",
        "elif           ",
        "while          ",
        "for            ",
        "foreach        ",
        "return         ",
        "break          ",
        "continue       ",
        "null           ",
        "true           ",
        "false          ",
        "closure        ",
        "global         ",
        "try            ",
        "catch          ",
        "finally        ",
        "throw          ",
        "final          ",

        "identifier     ",
    };

    token_t tok;
    bool b = true;
    while ((tok = lexer_peek(lex))->type != TOKEN_TYPE_END) {
        if (b) {
            lexer_unget(lex, tok);
            b = false;
        }
        printf("(%ld, %ld)  \ttype: %s\tvalue: %s\ttoken: %s\n", 
               tok->line,
               tok->column,
               types[tok->type],
               values[tok->value], 
               tok->token);
        tok = lexer_next(lex);
    }
   
}

struct node {
    int i;
    list_node_t link;
}nodes[1000];

void func(list_t list)
{
    list_t l;
    int i = 0;
    for (; i < 100; i++) {
        nodes[i].i = i;
        list_push_back(list, nodes[i].link);
    }

    list_init(l);

    list_split(l, list, nodes[50].link);

    list_push_front_list(list, l);
}

int main(void)
{

#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);  
#endif

{
    source_code_t sc = source_code_new("test.u", SOURCE_CODE_TYPE_FILE);
    lexer_t lex = lexer_new(sc);
    parser_t parse = parser_new(lex);
    environment_t env = environment_new();
    executor_t exec;

    environment_add_module(env, parser_generate_module(parse));

    exec = executor_new(env);
    executor_run(exec);

    executor_free(exec);

    environment_free(env);
    parser_free(parse);
    lexer_free(lex);
    source_code_free(sc);
}
    return 0;
}
