

#include "parser.h"
#include "alloc.h"
#include "error.h"
#include "expr.h"

#include <assert.h>

struct parser_s {
    lexer_t lex;
};

static void __parser_translation_unit__(parser_t parse);
static void __parser_definition_or_statement__(parser_t parse);
static void __parser_function_definition__(parser_t parse);
static void __parser_parameter_list__(parser_t parse);
static void __parser_block__(parser_t parse);
static void __parser_statement_list__(parser_t parse);
static void __parser_statement__(parser_t parse);
static void __parser_global_statement__(parser_t parse);
static void __parser_identifier_list__(parser_t parse);
static void __parser_if_statement__(parser_t parse);
static void __parser_elif_statement__(parser_t parse);
static void __parser_else_statement__(parser_t parse);
static void __parser_while_statement__(parser_t parse);
static void __parser_for_statement__(parser_t parse);
static void __parser_return_statement__(parser_t parse);
static void __parser_break_statement__(parser_t parse);
static void __parser_continue_statement__(parser_t parse);
static expr_t __parser_expression__(parser_t parse);
static expr_t __parser_logical_or_expression__(parser_t parse);
static expr_t __parser_logical_and_expression__(parser_t parse);
static expr_t __parser_equality_expression__(parser_t parse);
static expr_t __parser_relational_expression__(parser_t parse);
static expr_t __parser_additive_expression__(parser_t parse);
static expr_t __parser_multiplicative_expression__(parser_t parse);
static expr_t __parser_unary_expression__(parser_t parse);
static expr_t __parser_primary_expression__(parser_t parse);
static void __parser_argument_list__(parser_t parse, list_t args);

parser_t parser_new(lexer_t lex)
{
    parser_t parse = (parser_t)heap_alloc(sizeof(struct parser_s));
    if (!parse) {
        return NULL;
    }

    parse->lex = lex;

    return parse;
}

void parser_free(parser_t parse)
{
    heap_free(parse);
}

void parser_translation(parser_t parse)
{
    __parser_translation_unit__(parse);
}

static void __parser_need__(parser_t parse, token_value_t tv, const char *emsg)
{
    token_t tok = lexer_peek(parse->lex);
    if (tok->value != tv) {
        expect(tok->filename, tok->line, tok->column, emsg);
    }
}

static void __parser_expect__(parser_t parse, token_value_t tv, const char *emsg)
{
    token_t tok = lexer_peek(parse->lex);
    if (tok->value != tv) {
        expect(tok->filename, tok->line, tok->column, emsg);
    }
    lexer_next(parse->lex);
}

static void __parser_expect_next__(parser_t parse, token_value_t tv, const char *emsg)
{
    token_t tok = lexer_next(parse->lex);
    if (tok->value != tv) {
        expect(tok->filename, tok->line, tok->column, emsg);
    }
}

static void __parser_translation_unit__(parser_t parse)
{
    while (lexer_peek(parse->lex)->type != TOKEN_TYPE_END) {
        __parser_definition_or_statement__(parse);
    }
}

static void __parser_definition_or_statement__(parser_t parse)
{
    token_t tok;
    
    tok = lexer_peek(parse->lex);

    if (tok->value == TOKEN_VALUE_FUNCTION) {
        __parser_function_definition__(parse);
    } else {

    }
}

static void __parser_function_definition__(parser_t parse)
{
    __parser_expect_next__(parse, TOKEN_VALUE_IDENTIFIER,
        "expected a function name");

    __parser_expect_next__(parse, TOKEN_VALUE_LP, 
        "expected '(' after 'function'");

    lexer_next(parse->lex);

    __parser_parameter_list__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RP,
        "expected ')'");
    
    __parser_block__(parse);
}

static void __parser_parameter_list__(parser_t parse)
{
    token_t tok = lexer_peek(parse->lex);

    while (tok->type != TOKEN_TYPE_END && tok->value == TOKEN_VALUE_IDENTIFIER) {
        tok = lexer_next(parse->lex);
        if (tok->value != TOKEN_VALUE_COMMA) {
            break;
        }
        tok = lexer_next(parse->lex);
    }
}

static void __parser_block__(parser_t parse)
{
    __parser_expect__(parse, TOKEN_VALUE_LC, "expected '{'");

    __parser_statement_list__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RC, "expected '}'");
}

static void __parser_statement_list__(parser_t parse)
{
    token_t tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RC) {
        __parser_statement__(parse);
    }
}

static void __parser_statement__(parser_t parse)
{
    switch (lexer_peek(parse->lex)->value) {
    case TOKEN_VALUE_GLOBAL:
        __parser_global_statement__(parse);
        break;
    case TOKEN_VALUE_IF:
        __parser_if_statement__(parse);
        break;
    case TOKEN_VALUE_WHILE:
        __parser_while_statement__(parse);
        break;
    case TOKEN_VALUE_FOR:
        __parser_for_statement__(parse);
        break;
    case TOKEN_VALUE_RETURN:
        __parser_return_statement__(parse);
        break;
    case TOKEN_VALUE_BREAK:
        __parser_break_statement__(parse);
        break;
    case TOKEN_VALUE_CONTINUE:
        __parser_continue_statement__(parse);
        break;
    default:
        __parser_expression__(parse);
        break;
    }
}

static void __parser_global_statement__(parser_t parse)
{
    __parser_expect_next__(parse, TOKEN_VALUE_IDENTIFIER, 
        "expected identifier after global");

    __parser_identifier_list__(parse);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of global statement");
}

static void __parser_identifier_list__(parser_t parse)
{
    token_t tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && TOKEN_VALUE_IDENTIFIER) {
        tok = lexer_next(parse->lex);

        if (tok->value == TOKEN_VALUE_COMMA) {
            break;
        }

        tok = lexer_next(parse->lex);
    }
}

static void __parser_if_statement__(parser_t parse)
{
    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after if");

    __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    __parser_block__(parse);

    switch (lexer_peek(parse->lex)->value) {
    case TOKEN_VALUE_ELIF:
        __parser_elif_statement__(parse);
        break;
    case TOKEN_VALUE_ELSE:
        __parser_else_statement__(parse);
        break;
    }
}

static void __parser_elif_statement__(parser_t parse)
{
    token_t tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && tok->value == TOKEN_VALUE_ELIF) {
        lexer_next(parse->lex);

        __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after elif");

        __parser_expression__(parse);

        __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

        __parser_block__(parse);
    }

    if (lexer_peek(parse->lex)->value == TOKEN_VALUE_ELSE) {
        __parser_else_statement__(parse);
    }
}

static void __parser_else_statement__(parser_t parse)
{
    lexer_next(parse->lex);
    __parser_block__(parse);
}

static void __parser_while_statement__(parser_t parse)
{
    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after while");

    __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    __parser_block__(parse);
}

static void __parser_for_statement__(parser_t parse)
{
    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after for");

    __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';'");

    __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';'");

    __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    __parser_block__(parse);
}

static void __parser_return_statement__(parser_t parse)
{
    lexer_next(parse->lex);

    __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of return statement");
}

static void __parser_break_statement__(parser_t parse)
{
    lexer_next(parse->lex);
    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of break statement");
}

static void __parser_continue_statement__(parser_t parse)
{
    lexer_next(parse->lex);
    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of continue statement");
}

static expr_t __parser_expression__(parser_t parse)
{
    return __parser_logical_or_expression__(parse);
}

static expr_t __parser_logical_or_expression__(parser_t parse)
{
    expr_t left, right, binary = NULL;
    __parser_logical_and_expression__(parse);
    while (lexer_peek(parse->lex)->value == TOKEN_VALUE_OR) {
        lexer_next(parse->lex);
        __parser_logical_and_expression__(parse);
    }
    return binary;
}

static expr_t __parser_logical_and_expression__(parser_t parse)
{
    expr_t left, right, binary = NULL;
    __parser_equality_expression__(parse);
    while (lexer_peek(parse->lex)->value == TOKEN_VALUE_AND) {
        lexer_next(parse->lex);
        __parser_equality_expression__(parse);
    }
    return binary;
}

static expr_t __parser_equality_expression__(parser_t parse)
{
    token_t tok;
    expr_t left, right, binary = NULL;
    __parser_relational_expression__(parse);
    tok = lexer_peek(parse->lex);
    while (tok->value == TOKEN_VALUE_EQ || tok->value == TOKEN_VALUE_NEQ) {
        lexer_next(parse->lex);
        __parser_relational_expression__(parse);
    }
    return binary;
}

static expr_t __parser_relational_expression__(parser_t parse)
{
    token_t tok;
    expr_t left, right, binary = NULL;
    __parser_additive_expression__(parse);
    tok = lexer_peek(parse->lex);
    while (tok->value == TOKEN_VALUE_GT || 
           tok->value == TOKEN_VALUE_GEQ || 
           tok->value == TOKEN_VALUE_LT || 
           tok->value == TOKEN_VALUE_LEQ) {
        lexer_next(parse->lex);
        __parser_additive_expression__(parse);
    }
    return binary;
}

static expr_t __parser_additive_expression__(parser_t parse)
{
    token_t tok;
    expr_t left, right, binary = NULL;
    __parser_multiplicative_expression__(parse);
    tok = lexer_peek(parse->lex);
    while (tok->value == TOKEN_VALUE_ADD || tok->value == TOKEN_VALUE_SUB) {
        lexer_next(parse->lex);
        __parser_multiplicative_expression__(parse);
    }
    return binary;
}

static expr_t __parser_multiplicative_expression__(parser_t parse)
{
    token_t tok;
    expr_t left, right, binary = NULL;
    __parser_unary_expression__(parse);
    tok = lexer_peek(parse->lex);
    while (tok->value == TOKEN_VALUE_MUL ||
           tok->value == TOKEN_VALUE_DIV || 
           tok->value == TOKEN_VALUE_MOD) {
        lexer_next(parse->lex);
        __parser_unary_expression__(parse);
    }
    return binary;
}

static expr_t __parser_unary_expression__(parser_t parse)
{
    expr_t expr = NULL;
    token_t tok = lexer_peek(parse->lex);
    switch (tok->value) {
    case TOKEN_VALUE_ADD:
        lexer_next(parse->lex);
        expr = expr_new_plus(tok->line, tok->column, __parser_primary_expression__(parse));
        break;
    case TOKEN_VALUE_SUB:
        lexer_next(parse->lex);
        expr = expr_new_minus(tok->line, tok->column, __parser_primary_expression__(parse));
        break;
    default:
        expr = __parser_primary_expression__(parse);
        break;
    }
    assert(expr != NULL);
    return expr;
}

static expr_t __parser_primary_expression__(parser_t parse)
{
    long line, column;
    token_t tok;
    cstring_t identifier;
    expr_t expr = NULL;

    tok = lexer_peek(parse->lex);
    switch (tok->value) {
    case TOKEN_VALUE_IDENTIFIER:
        identifier = cstring_dup(tok->token);
        line = tok->line;
        column = tok->column;

        switch (lexer_next(parse->lex)->value) {
        case TOKEN_VALUE_LP:
            expr = expr_new_call(line, column, identifier);
            __parser_argument_list__(parse, expr->u.call.args);
            __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");
            break;

        case TOKEN_VALUE_ASSIGN:
            lexer_next(parse->lex);
            expr = expr_new_assign(line, column, identifier, __parser_expression__(parse));
            __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, "expected ';'");
            break;

        default:
            expr = expr_new_identifier(line, column, identifier);
            break;
        }

        break;

    case TOKEN_VALUE_LP:
        lexer_next(parse->lex);
        expr = __parser_expression__(parse);
        __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");
        break;

    case TOKEN_VALUE_LITERAL_CHAR:
        expr = expr_new(EXPR_TYPE_CHAR, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_INT:
        expr = expr_new(EXPR_TYPE_INT, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_LONG:
        expr = expr_new(EXPR_TYPE_LONG, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_FLOAT:
        expr = expr_new(EXPR_TYPE_FLOAT, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_DOUBLE:
        expr = expr_new(EXPR_TYPE_DOUBLE, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_STRING:
        expr = expr_new(EXPR_TYPE_DOUBLE, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_TRUE:
        expr = expr_new(EXPR_TYPE_BOOL, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_FALSE:
        expr = expr_new(EXPR_TYPE_BOOL, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_NULL:
        expr = expr_new(EXPR_TYPE_NULL, tok);
        lexer_next(parse->lex);
        break;

    default:
        expect(tok->filename, tok->line, tok->column, 
            "expected expression");
        break;
    }

    assert(expr != NULL);
    return expr;
}

static void __parser_argument_list__(parser_t parse, list_t args)
{
    token_t tok = lexer_next(parse->lex);
    expr_t expr;
    while (tok->type != TOKEN_TYPE_END && tok->value == TOKEN_VALUE_RP) {
        lexer_next(parse->lex);

        expr = __parser_expression__(parse);
      
        if (lexer_peek(parse->lex)->value != TOKEN_VALUE_COMMA) {
            break;
        }
        lexer_next(parse->lex);
    }
}