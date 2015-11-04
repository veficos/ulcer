

#include "parser.h"
#include "module.h"
#include "alloc.h"
#include "error.h"
#include "expr.h"
#include "stmt.h"

#include <assert.h>

struct parser_s {
    lexer_t  lex;
    module_t module;
};

static void __parser_translation_unit__(parser_t parse);
static void __parser_definition_or_statement__(parser_t parse);
static function_t __parser_function_definition__(parser_t parse);
static void __parser_parameter_list__(parser_t parse, function_t func);
static stmt_t __parser_block__(parser_t parse);
static stmt_t __parser_statement__(parser_t parse);
static stmt_t __parser_global_statement__(parser_t parse);
static void __parser_identifier_list__(parser_t parse, list_t vars);
static stmt_t __parser_if_statement__(parser_t parse);
static void __parser_elif_statement__(parser_t parse, stmt_if_t* if_stmt);
static stmt_t __parser_else_statement__(parser_t parse);
static stmt_t __parser_while_statement__(parser_t parse);
static stmt_t __parser_for_statement__(parser_t parse);
static stmt_t __parser_return_statement__(parser_t parse);
static stmt_t __parser_break_statement__(parser_t parse);
static stmt_t __parser_continue_statement__(parser_t parse);
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
    parser_t parse = (parser_t)mem_alloc(sizeof(struct parser_s));
    if (!parse) {
        return NULL;
    }

    parse->module    = module_new();
    parse->lex       = lex;

    return parse;
}

void parser_free(parser_t parse)
{
    module_free(parse->module);
    mem_free(parse);
}

module_t parser_generate_module(parser_t parse)
{
    __parser_translation_unit__(parse);

    return parse->module;
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
    long line, column;
    function_t func;
    stmt_t stmt;

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;
    if (tok->value == TOKEN_VALUE_FUNCTION) {
        func = __parser_function_definition__(parse);
        if (!module_add_function(parse->module, func)) {
            error(tok->filename, line, column, "redefinition function of '%s'", func->name);
        }

    } else {
        stmt = __parser_statement__(parse);
        module_add_statment(parse->module, stmt);
    }
}

static function_t __parser_function_definition__(parser_t parse)
{
    token_t tok;
    long line, column;
    function_t func = NULL;

    tok      = lexer_peek(parse->lex);
    line     = tok->line;
    column   = tok->column;

    __parser_expect_next__(parse, TOKEN_VALUE_IDENTIFIER,
        "expected a function name");

    func = function_new_self(line, column, cstring_dup(lexer_peek(parse->lex)->token));

    __parser_expect_next__(parse, TOKEN_VALUE_LP, 
        "expected '(' after 'function'");

    lexer_next(parse->lex);

    __parser_parameter_list__(parse, func);

    __parser_expect__(parse, TOKEN_VALUE_RP,
        "expected ')'");
    
    func->u.self.block = __parser_block__(parse);

    return func;
}

static void __parser_parameter_list__(parser_t parse, function_t func)
{
    token_t tok = lexer_peek(parse->lex);

    while (tok->type != TOKEN_TYPE_END && tok->value == TOKEN_VALUE_IDENTIFIER) {
        list_push_back(func->u.self.parameters, parameter_new(cstring_dup(tok->token))->link);
        tok = lexer_next(parse->lex);
        if (tok->value != TOKEN_VALUE_COMMA) {
            break;
        }
        tok = lexer_next(parse->lex);
    }
}

static stmt_t __parser_block__(parser_t parse)
{
    token_t tok;
    stmt_t stmt = NULL;
    
    tok  = lexer_peek(parse->lex);

    stmt = stmt_new_block(tok->line, tok->column);

    __parser_expect__(parse, TOKEN_VALUE_LC, "expected '{'");

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RC) {
        list_push_back(stmt->u.block, __parser_statement__(parse)->link);
    }

    __parser_expect__(parse, TOKEN_VALUE_RC, "expected '}'");

    return stmt;
}

static stmt_t __parser_statement__(parser_t parse)
{
    stmt_t stmt = NULL;
    long line, column;
    token_t tok;
    
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    switch (tok->value) {
    case TOKEN_VALUE_GLOBAL:
        stmt = __parser_global_statement__(parse);
        break;

    case TOKEN_VALUE_IF:
        stmt = __parser_if_statement__(parse);
        break;

    case TOKEN_VALUE_WHILE:
        stmt = __parser_while_statement__(parse);
        break;

    case TOKEN_VALUE_FOR:
        stmt = __parser_for_statement__(parse);
        break;

    case TOKEN_VALUE_RETURN:
        stmt = __parser_return_statement__(parse);
        break;

    case TOKEN_VALUE_BREAK:
        stmt = __parser_break_statement__(parse);
        break;

    case TOKEN_VALUE_CONTINUE:
        stmt = __parser_continue_statement__(parse);
        break;

    default:
        stmt = stmt_new_expr(line, column, __parser_expression__(parse));
        __parser_expect__(parse, TOKEN_VALUE_SEMICOLON,
            "expected ';' at end of expression");
        break;
    }

    assert(stmt != NULL);
    return stmt;
}

static stmt_t __parser_global_statement__(parser_t parse)
{
    stmt_t stmt = NULL;
    long line, column;
    token_t tok;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    __parser_expect_next__(parse, TOKEN_VALUE_IDENTIFIER, 
        "expected identifier after global");

    stmt = stmt_new_global(line, column);

    __parser_identifier_list__(parse, stmt->u.stmt_global);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of global statement");

    assert(stmt != NULL);
    return stmt;
}

static void __parser_identifier_list__(parser_t parse, list_t vars)
{
    token_t tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && TOKEN_VALUE_IDENTIFIER) {
        stmt_global_t *global = mem_alloc(sizeof(stmt_global_t));

        global->name = cstring_dup(tok->token);
        list_push_back(vars, global->link);

        tok = lexer_next(parse->lex);

        if (tok->value != TOKEN_VALUE_COMMA) {
            break;
        }

        tok = lexer_next(parse->lex);
    }
}

static stmt_t __parser_if_statement__(parser_t parse)
{
    long line, column;
    token_t tok;
    expr_t condition;
    stmt_t if_stmt = NULL;
    stmt_t if_block;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after if");

    condition = __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    if_block = __parser_block__(parse);

    if_stmt = stmt_new_if(line, column, condition, if_block);

    switch (lexer_peek(parse->lex)->value) {
    case TOKEN_VALUE_ELIF:
        __parser_elif_statement__(parse, &if_stmt->u.stmt_if);
        break;
    case TOKEN_VALUE_ELSE:
        if_stmt->u.stmt_if.else_block = __parser_else_statement__(parse);
        break;
    }

    assert(if_stmt != NULL);
    return if_stmt;
}

static void __parser_elif_statement__(parser_t parse, stmt_if_t* if_stmt)
{
    long line, column;
    token_t tok;
    expr_t condition;
    stmt_t elif;
    
    tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && tok->value == TOKEN_VALUE_ELIF) {
        line    = tok->line;
        column  = tok->column;

        lexer_next(parse->lex);

        __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after elif");

        condition = __parser_expression__(parse);

        __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

        elif = stmt_new_elif(line, column, condition, __parser_block__(parse));

        list_push_back(if_stmt->elifs, elif->link);
    }

    if (lexer_peek(parse->lex)->value == TOKEN_VALUE_ELSE) {
        if_stmt->else_block = __parser_else_statement__(parse);
    }
}

static stmt_t __parser_else_statement__(parser_t parse)
{
    stmt_t stmt;
    
    lexer_next(parse->lex);
    stmt = __parser_block__(parse);

    assert(stmt != NULL);
    return stmt;
}

static stmt_t __parser_while_statement__(parser_t parse)
{
    long line, column;
    expr_t condition;
    token_t tok;
    stmt_t stmt = NULL;
    
    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after while");

    condition = __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    stmt = stmt_new_while(line, column, condition, __parser_block__(parse));

    assert(stmt != NULL);
    return stmt;
}

static stmt_t __parser_for_statement__(parser_t parse)
{
    expr_t init = NULL, condition = NULL, post = NULL;
    stmt_t stmt = NULL;
    long line, column;
    token_t tok;

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after for");

    if (lexer_peek(parse->lex)->value != TOKEN_VALUE_SEMICOLON) {
        init = __parser_expression__(parse);
    }

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';'");

    if (lexer_peek(parse->lex)->value != TOKEN_VALUE_SEMICOLON) {
        condition = __parser_expression__(parse);
    }

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';'");

    if (lexer_peek(parse->lex)->value != TOKEN_VALUE_RP) {
        post = __parser_expression__(parse);
    }

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    stmt = stmt_new_for(line, column, init, condition, post, __parser_block__(parse));

    assert(stmt != NULL);
    return stmt;
}

static stmt_t __parser_return_statement__(parser_t parse)
{
    expr_t expr;
    long line, column;
    token_t tok = lexer_peek(parse->lex);

    line = tok->line;
    column = tok->column;

    lexer_next(parse->lex);

    expr = __parser_expression__(parse);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of return statement");

    return stmt_new_return(line, column, expr);
}

static stmt_t __parser_break_statement__(parser_t parse)
{
    long line, column;
    token_t tok = lexer_peek(parse->lex);

    line    = tok->line;
    column  = tok->column;

    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of break statement");

    return stmt_new_break(line, column);
}

static stmt_t __parser_continue_statement__(parser_t parse)
{
    long line, column;
    token_t tok = lexer_peek(parse->lex);

    line    = tok->line;
    column  = tok->column;

    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, 
        "expected ';' at end of continue statement");

    return stmt_new_continue(line, column);
}

static expr_t __parser_expression__(parser_t parse)
{
    expr_t expr = __parser_logical_or_expression__(parse);
    assert(expr != NULL);
    return expr;
}

static expr_t __parser_logical_or_expression__(parser_t parse)
{
    token_t tok;
    long line, column;
    expr_t expr = NULL;

    expr    = __parser_logical_and_expression__(parse);
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;
    while (lexer_peek(parse->lex)->value == TOKEN_VALUE_OR) {
        lexer_next(parse->lex);
        expr    = expr_new_binary(EXPR_TYPE_OR, line, column, expr, __parser_logical_and_expression__(parse));
        tok     = lexer_peek(parse->lex);
        line    = tok->line;
        column  = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expr_t __parser_logical_and_expression__(parser_t parse)
{
    token_t tok;
    long line, column;
    expr_t expr = NULL;

    expr   = __parser_equality_expression__(parse);
    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;
    while (tok->value == TOKEN_VALUE_AND) {
        lexer_next(parse->lex);
        expr   = expr_new_binary(EXPR_TYPE_AND, line, column, expr, __parser_equality_expression__(parse));
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expr_t __parser_equality_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expr_t expr = NULL;

    expr   = __parser_relational_expression__(parse);
    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;
    while (tok->value == TOKEN_VALUE_EQ || 
           tok->value == TOKEN_VALUE_NEQ) {
        tv  = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_EQ:
            expr = expr_new_binary(EXPR_TYPE_EQ, line, column, expr, __parser_relational_expression__(parse));
            break;

        case TOKEN_VALUE_NEQ:
            expr = expr_new_binary(EXPR_TYPE_NEQ, line, column, expr, __parser_relational_expression__(parse));
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expr_t __parser_relational_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expr_t expr = NULL;

    expr   = __parser_additive_expression__(parse);
    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;
    while (tok->value == TOKEN_VALUE_GT || 
           tok->value == TOKEN_VALUE_GEQ || 
           tok->value == TOKEN_VALUE_LT || 
           tok->value == TOKEN_VALUE_LEQ) {
        tv  = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_GT:
            expr = expr_new_binary(EXPR_TYPE_GT, line, column, expr, __parser_additive_expression__(parse));
            break;

        case TOKEN_VALUE_GEQ:
            expr = expr_new_binary(EXPR_TYPE_GEQ, line, column, expr, __parser_additive_expression__(parse));
            break;

        case TOKEN_VALUE_LT:
            expr = expr_new_binary(EXPR_TYPE_LT, line, column, expr, __parser_additive_expression__(parse));
            break;

        case TOKEN_VALUE_LEQ:
            expr = expr_new_binary(EXPR_TYPE_LEQ, line, column, expr, __parser_additive_expression__(parse));
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expr_t __parser_additive_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expr_t expr = NULL;

    expr    = __parser_multiplicative_expression__(parse);
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;
    while (tok->value == TOKEN_VALUE_ADD || 
           tok->value == TOKEN_VALUE_SUB) {
        tv = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_ADD:
            expr = expr_new_binary(EXPR_TYPE_ADD, line, column, expr, __parser_multiplicative_expression__(parse));
            break;

        case TOKEN_VALUE_SUB:
            expr = expr_new_binary(EXPR_TYPE_SUB, line, column, expr, __parser_multiplicative_expression__(parse));
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expr_t __parser_multiplicative_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expr_t expr = NULL;

    expr    = __parser_unary_expression__(parse);
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;
    while (tok->value == TOKEN_VALUE_MUL ||
           tok->value == TOKEN_VALUE_DIV || 
           tok->value == TOKEN_VALUE_MOD) {
        tv      = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_MUL:
            expr = expr_new_binary(EXPR_TYPE_MUL, line, column, expr, __parser_unary_expression__(parse));
            break;

        case TOKEN_VALUE_DIV:
            expr = expr_new_binary(EXPR_TYPE_DIV, line, column, expr, __parser_unary_expression__(parse));
            break;

        case TOKEN_VALUE_MOD:
            expr = expr_new_binary(EXPR_TYPE_MOD, line, column, expr, __parser_unary_expression__(parse));
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
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
        expr = expr_new(EXPR_TYPE_STRING, tok);
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
    expr_t expr = NULL;
    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RP) {
        expr = __parser_expression__(parse);
        
        assert(expr != NULL);

        list_push_back(args, expr->link);
      
        if (lexer_peek(parse->lex)->value != TOKEN_VALUE_COMMA) {
            break;
        }

        lexer_next(parse->lex);
    }
}
