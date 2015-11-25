

#include "parser.h"
#include "module.h"
#include "alloc.h"
#include "error.h"

#include <assert.h>

struct parser_s {
    lexer_t  lex;
    module_t module;
};

static void         __parser_translation_unit__(parser_t parse);
static void         __parser_toplevel_statement__(parser_t parse);
static statement_t  __parser_require_statement__(parser_t parse);
static expression_t __parser_function_definition__(parser_t parse);
static statement_t  __parser_statement__(parser_t parse);
static expression_t __parser_expression__(parser_t parse);
static expression_t __parser_lvalue_expression__(parser_t parse);
static expression_t __parser_rvalue_expression__(parser_t parse);
static expression_t __parser_logical_or_expression__(parser_t parse);
static expression_t __parser_logical_and_expression__(parser_t parse);
static expression_t __parser_equality_expression__(parser_t parse);
static expression_t __parser_relational_expression__(parser_t parse);
static expression_t __parser_additive_expression__(parser_t parse);
static expression_t __parser_multiplicative_expression__(parser_t parse);
static expression_t __parser_bitop_expression__(parser_t parse);
static expression_t __parser_shift_bitop_expression__(parser_t parse);
static expression_t __parser_unary_expression__(parser_t parse);
static expression_t __parser_postfix_expression__(parser_t parse);
static expression_t __parser_primary_expression__(parser_t parse);
static expression_t __parser_array_generate_expression__(parser_t parse);
static expression_t __parser_table_generate_expression__(parser_t parse);
static expression_t __parser_closure_definition__(parser_t parse);
static list_t       __parser_parameter_list__(parser_t parse);
static list_t       __parser_argument_list__(parser_t parse);
static list_t       __parser_block__(parser_t parse);

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
        error(tok->filename, tok->line, tok->column, emsg);
    }
}

static void __parser_expect__(parser_t parse, token_value_t tv, const char *emsg)
{
    token_t tok = lexer_peek(parse->lex);
    if (tok->value != tv) {
        error(tok->filename, tok->line, tok->column, emsg);
    }
    lexer_next(parse->lex);
}

static void __parser_expect_next__(parser_t parse, token_value_t tv, const char *emsg)
{
    token_t tok = lexer_next(parse->lex);
    if (tok->value != tv) {
        error(tok->filename, tok->line, tok->column, emsg);
    }
}

static void __parser_translation_unit__(parser_t parse)
{
    while (lexer_peek(parse->lex)->type != TOKEN_TYPE_END) {
        __parser_toplevel_statement__(parse);
    }
}

static void __parser_toplevel_statement__(parser_t parse)
{
    token_t tok;
    long line, column;
    statement_t stmt;
    expression_t expr;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    switch (tok->value) {
    case TOKEN_VALUE_REQUIRE:
        stmt = __parser_require_statement__(parse);
        module_add_statment(parse->module, stmt);
        break;

    case TOKEN_VALUE_FUNCTION:
        expr = __parser_function_definition__(parse);

        module_add_statment(parse->module, statement_new_expression(line, column, expr));
        break;

    default:
        stmt = __parser_statement__(parse);
        if (stmt) {
            module_add_statment(parse->module, stmt);
        }
        break;
    }
}

static statement_t __parser_require_statement__(parser_t parse)
{
    statement_t stmt = NULL;
    token_t tok  = NULL;
    long line, column;

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    __parser_expect_next__(parse, TOKEN_VALUE_LITERAL_STRING, "require expects \"FILENAME\"");

    stmt = statement_new_require(line, column, cstring_dup(lexer_peek(parse->lex)->token));

    lexer_next(parse->lex);

    assert(stmt != NULL);
    return stmt;
}

static expression_t __parser_function_definition__(parser_t parse)
{
    expression_t expr;
    long line, column;
    cstring_t funcname;
    list_t parameters;
    list_t block;
    token_t tok;

    expr      = NULL;
    tok       = lexer_next(parse->lex);
    line      = tok->line;
    column    = tok->column;

    if (tok->value == TOKEN_VALUE_IDENTIFIER) {
        funcname = cstring_dup(tok->token);
        lexer_next(parse->lex);
    } else {
        funcname = NULL;
    }

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after 'function'");

    parameters = __parser_parameter_list__(parse);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    block = __parser_block__(parse);

    expr = expression_new_function(line, column, funcname, parameters, block);

    assert(expr != NULL);
    return expr;
}

static statement_t __parser_statement__(parser_t parse)
{
    expression_t expr;
    statement_t stmt;
    long line, column;
    token_t tok;

    stmt    = NULL;
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    switch (tok->value) {
    case TOKEN_VALUE_IF:
        break;

    case TOKEN_VALUE_WHILE:
        break;

    case TOKEN_VALUE_FOR:
        break;

    case TOKEN_VALUE_RETURN:
        break;

    case TOKEN_VALUE_BREAK:
        break;

    case TOKEN_VALUE_CONTINUE:
        break;

    case TOKEN_VALUE_SEMICOLON:
        lexer_next(parse->lex);
        break;

    default:
        expr = __parser_expression__(parse);
        stmt = statement_new_expression(line, column, expr);
        assert(stmt != NULL);
        if (lexer_peek(parse->lex)->value == TOKEN_VALUE_SEMICOLON) {
            lexer_next(parse->lex);
        }
        break;
    }

    return stmt;
}

static expression_t __parser_expression__(parser_t parse)
{
    expression_t expr;
    expression_type_t expt_type;
    list_t lvalue_exprs;
    list_t rvalue_exprs;
    long line, column;
    token_t tok;
    int lvalue_exprs_total;
    int rvalue_exprs_total;

    expt_type = EXPRESSION_TYPE_ASSIGN;

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    list_init(lvalue_exprs);
    list_init(rvalue_exprs);

    /* left value expression list */
    for (lvalue_exprs_total = 0; (expr = __parser_lvalue_expression__(parse)) != NULL; ) {
        list_push_back(lvalue_exprs, expr->link);
        
        lvalue_exprs_total++;

        if (lexer_peek(parse->lex)->value == TOKEN_VALUE_COMMA) {
            lexer_next(parse->lex);
        } else {
            break;
        }
    }

    tok = lexer_peek(parse->lex);
    switch (tok->value) {
    case TOKEN_VALUE_ASSIGN:
        if (lvalue_exprs_total == 0) {
            error(tok->filename, line, column, "expected lvalue expression");
        }

        lexer_next(parse->lex);

        /* right value expression list */
        for (rvalue_exprs_total = 0; (expr = __parser_rvalue_expression__(parse)) != NULL ; ) {
            list_push_back(rvalue_exprs, expr->link);
            
            rvalue_exprs_total++;

            if (lexer_peek(parse->lex)->value == TOKEN_VALUE_COMMA) {
                lexer_next(parse->lex);
            } else {
                break;
            }
        }

        if (rvalue_exprs_total == 0) {
            error(tok->filename, line, column, "expected rvalue expression");
        }

        expr = expression_new_assign(line, column, lvalue_exprs, lvalue_exprs_total, rvalue_exprs, rvalue_exprs_total);

        goto done;
        break;

    case TOKEN_VALUE_ADD_ASSIGN:
        expt_type = EXPRESSION_TYPE_ADD_ASSIGN;
        break;

    case TOKEN_VALUE_SUB_ASSIGN:
        expt_type = EXPRESSION_TYPE_SUB_ASSIGN;
        break;

    case TOKEN_VALUE_MUL_ASSIGN:
        expt_type = EXPRESSION_TYPE_MUL_ASSIGN;
        break;

    case TOKEN_VALUE_DIV_ASSIGN:
        expt_type = EXPRESSION_TYPE_DIV_ASSIGN;
        break;

    case TOKEN_VALUE_MOD_ASSIGN:
        expt_type = EXPRESSION_TYPE_MOD_ASSIGN;
        break;

    case TOKEN_VALUE_BITAND_ASSIGN:
        expt_type = EXPRESSION_TYPE_BITAND_ASSIGN;
        break;

    case TOKEN_VALUE_BITOR_ASSIGN:
        expt_type = EXPRESSION_TYPE_BITOR_ASSIGN;
        break;

    case TOKEN_VALUE_XOR_ASSIGN:
        expt_type = EXPRESSION_TYPE_XOR_ASSIGN;
        break;

    case TOKEN_VALUE_LEFI_SHIFT_ASSIGN:
        expt_type = EXPRESSION_TYPE_LEFI_SHIFT_ASSIGN;
        break;

    case TOKEN_VALUE_RIGHT_SHIFT_ASSIGN:
        expt_type = EXPRESSION_TYPE_RIGHT_SHIFT_ASSIGN;
        break;

    case TOKEN_VALUE_LOGIC_RIGHT_SHIFT_ASSIGN:
        expt_type = EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT_ASSIGN;
        break;

    default:
        if (lvalue_exprs_total == 1 && (expr = list_element(list_begin(lvalue_exprs), expression_t, link))->type == EXPRESSION_TYPE_CALL) {
            goto done;
        }
        
        error(tok->filename, line, column, "unexpected assign expression");
        break;
    }

    /* component assign expression */
    if (lvalue_exprs_total > 1) {
        error(tok->filename, line, column, "unexpected assign expression");
    }

    lexer_next(parse->lex);

    expr = __parser_rvalue_expression__(parse);
    if (expr == NULL) {
        error(tok->filename, line, column, "expected rvalue expression");
    }

    expr = expression_new_component_assign(line, column, expt_type, 
        list_element(list_begin(lvalue_exprs), expression_t, link), expr);

done:
    assert (expr != NULL);
    return expr;
}

static expression_t __parser_lvalue_expression__(parser_t parse)
{
    expression_t lvalue_expr;
    long line, column;
    token_t tok;
    
    lvalue_expr = NULL;
    tok         = lexer_peek(parse->lex);
    line        = tok->line;
    column      = tok->column;

    switch (tok->value) {
    case TOKEN_VALUE_IGNORE:
        lvalue_expr = expression_new_ignore(line, column);
        lexer_next(parse->lex);
        break;

    default:
        lvalue_expr = __parser_postfix_expression__(parse);
        break;
    }

    return lvalue_expr;
}

static expression_t __parser_rvalue_expression__(parser_t parse)
{
    return __parser_logical_or_expression__(parse);
}

static expression_t __parser_logical_or_expression__(parser_t parse)
{
    token_t tok;
    long line, column;
    expression_t expr = NULL;

    expr    = __parser_logical_and_expression__(parse);
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    while (tok->value == TOKEN_VALUE_OR) {
        lexer_next(parse->lex);
        expr    = expression_new_binary(line, column, EXPRESSION_TYPE_OR, expr, __parser_logical_and_expression__(parse));
        tok     = lexer_peek(parse->lex);
        line    = tok->line;
        column  = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_logical_and_expression__(parser_t parse)
{
    token_t tok;
    long line, column;
    expression_t expr = NULL;

    expr   = __parser_equality_expression__(parse);
    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    while (tok->value == TOKEN_VALUE_AND) {
        lexer_next(parse->lex);
        expr   = expression_new_binary(line, column, EXPRESSION_TYPE_AND, expr, __parser_equality_expression__(parse));
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_equality_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expression_t expr = NULL;

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
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_EQ, expr, __parser_relational_expression__(parse));
            break;

        case TOKEN_VALUE_NEQ:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_NEQ, expr, __parser_relational_expression__(parse));
            break;

        default:
            assert(false);
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_relational_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expression_t expr = NULL;

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
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_GT, expr, __parser_additive_expression__(parse));
            break;

        case TOKEN_VALUE_GEQ:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_GEQ, expr, __parser_additive_expression__(parse));
            break;

        case TOKEN_VALUE_LT:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_LT, expr, __parser_additive_expression__(parse));
            break;

        case TOKEN_VALUE_LEQ:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_LEQ, expr, __parser_additive_expression__(parse));
            break;

        default:
            assert(false);
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_additive_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expression_t expr = NULL;

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
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_ADD, expr, __parser_multiplicative_expression__(parse));
            break;

        case TOKEN_VALUE_SUB:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_SUB, expr, __parser_multiplicative_expression__(parse));
            break;

        default:
            assert(false);
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_multiplicative_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expression_t expr = NULL;

    expr    = __parser_bitop_expression__(parse);
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
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_MUL, expr, __parser_bitop_expression__(parse));
            break;

        case TOKEN_VALUE_DIV:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_DIV, expr, __parser_bitop_expression__(parse));
            break;

        case TOKEN_VALUE_MOD:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_MOD, expr, __parser_bitop_expression__(parse));
            break;

        default:
            assert(false);
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_bitop_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expression_t expr = NULL;

    expr    = __parser_unary_expression__(parse);
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    while (tok->value == TOKEN_VALUE_BITAND ||
           tok->value == TOKEN_VALUE_BITOR || 
           tok->value == TOKEN_VALUE_XOR) {
        tv      = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_BITAND:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_BITAND, expr, __parser_shift_bitop_expression__(parse));
            break;

        case TOKEN_VALUE_BITOR:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_BITOR, expr, __parser_shift_bitop_expression__(parse));
            break;

        case TOKEN_VALUE_XOR:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_XOR, expr, __parser_shift_bitop_expression__(parse));
            break;

        default:
            assert(false);
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_shift_bitop_expression__(parser_t parse)
{
    token_t tok;
    token_value_t tv;
    long line, column;
    expression_t expr = NULL;

    expr    = __parser_unary_expression__(parse);
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    while (tok->value == TOKEN_VALUE_LEFT_SHIFT  ||
           tok->value == TOKEN_VALUE_RIGHT_SHIFT || 
           tok->value == TOKEN_VALUE_LOGIC_RIGHT_SHIFT_ASSIGN) {
        tv      = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_LEFT_SHIFT:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_LEFT_SHIFT, expr, __parser_unary_expression__(parse));
            break;

        case TOKEN_VALUE_RIGHT_SHIFT:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_RIGHT_SHIFT, expr, __parser_unary_expression__(parse));
            break;

        case TOKEN_VALUE_LOGIC_RIGHT_SHIFT_ASSIGN:
            expr = expression_new_binary(line, column, EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT, expr, __parser_unary_expression__(parse));
            break;

        default:
            assert(false);
            break;
        }

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_unary_expression__(parser_t parse)
{
    expression_type_t expr_type;
    expression_t expr;
    long line, column;
    token_t tok;

    expr      = NULL;
    tok       = lexer_peek(parse->lex);
    line      = tok->line;
    column    = tok->column;

    switch (tok->value) {
    case TOKEN_VALUE_ADD:
        lexer_next(parse->lex);
        expr_type = EXPRESSION_TYPE_PLUS;
        break;

    case TOKEN_VALUE_SUB:
        lexer_next(parse->lex);
        expr_type = EXPRESSION_TYPE_MINUS;
        break;

    case TOKEN_VALUE_NOT:
        lexer_next(parse->lex);
        expr_type = EXPRESSION_TYPE_NOT;
        break;

    case TOKEN_VALUE_CPL:
        lexer_next(parse->lex);
        expr_type = EXPRESSION_TYPE_CPL;
        break;

    default:
        expr = __parser_postfix_expression__(parse);
        goto done;
        break;
    }

    expr = expression_new_unary(line, column, expr_type, __parser_unary_expression__(parse));

done:
    assert(expr != NULL);
    return expr;
}

static expression_t __parser_postfix_expression__(parser_t parse)
{
    expression_t expr = NULL;
    long line, column;
    token_t tok;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    expr = __parser_primary_expression__(parse);

    tok  = lexer_peek(parse->lex);
    while (tok->value == TOKEN_VALUE_LB     ||
           tok->value == TOKEN_VALUE_APPEND ||
           tok->value == TOKEN_VALUE_DOT    ||
           tok->value == TOKEN_VALUE_LP     || 
           tok->value == TOKEN_VALUE_INC    ||
           tok->value == TOKEN_VALUE_DEC) {
        switch (tok->value) {
        case TOKEN_VALUE_LB:
            lexer_next(parse->lex);

            expr = expression_new_index(line, column, expr, __parser_rvalue_expression__(parse));

            __parser_expect__(parse, TOKEN_VALUE_RB, "expected ']'");
            break;

        case TOKEN_VALUE_APPEND:
            lexer_next(parse->lex);

            expr = expression_new_array_append(line, column, expr, __parser_rvalue_expression__(parse));
            break;

        case TOKEN_VALUE_DOT:
            __parser_expect_next__(parse, TOKEN_VALUE_IDENTIFIER, "expected member name");

            expr = expression_new_table_dot_member(line, column, expr, cstring_dup(tok->token));

            lexer_next(parse->lex);
            break;

        case TOKEN_VALUE_LP:
            expr = expression_new_call(line, column, expr, __parser_argument_list__(parse));
            break;

        case TOKEN_VALUE_INC:
            lexer_next(parse->lex);
            expr = expression_new_incdec(line, column, EXPRESSION_TYPE_INC, expr);
            break;

        case TOKEN_VALUE_DEC:
            lexer_next(parse->lex);
            expr = expression_new_incdec(line, column, EXPRESSION_TYPE_DEC, expr);
            break;

        default: 
            break;
        }

        tok = lexer_peek(parse->lex);
    }

    return expr;
}

static expression_t __parser_primary_expression__(parser_t parse)
{
    expression_t expr;
    long line, column;
    token_t tok;
    
    expr      = NULL;
    tok       = lexer_peek(parse->lex);
    line      = tok->line;
    column    = tok->column;
    
    switch (tok->value) {
    case TOKEN_VALUE_TRUE:
        expr = expression_new_literal(EXPRESSION_TYPE_BOOL, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_FALSE:
        expr = expression_new_literal(EXPRESSION_TYPE_BOOL, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_NULL:
        expr = expression_new_literal(EXPRESSION_TYPE_NULL, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_CHAR:
        expr = expression_new_literal(EXPRESSION_TYPE_CHAR, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_INT:
        expr = expression_new_literal(EXPRESSION_TYPE_INT, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_LONG:
        expr = expression_new_literal(EXPRESSION_TYPE_LONG, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_FLOAT:
        expr = expression_new_literal(EXPRESSION_TYPE_FLOAT, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_DOUBLE:
        expr = expression_new_literal(EXPRESSION_TYPE_DOUBLE, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_LITERAL_STRING:
        expr = expression_new_literal(EXPRESSION_TYPE_STRING, tok);
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_IDENTIFIER:
        expr = expression_new_identifier(line, column, cstring_dup(tok->token));
        lexer_next(parse->lex);
        break;

    case TOKEN_VALUE_FUNCTION:
        expr = __parser_closure_definition__(parse);
        break;

    case TOKEN_VALUE_LP:
        lexer_next(parse->lex);
        expr = __parser_rvalue_expression__(parse);
        __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");
        break;

    case TOKEN_VALUE_LB:
        expr = __parser_array_generate_expression__(parse);
        break;

    case TOKEN_VALUE_LC:
        expr = __parser_table_generate_expression__(parse);
        break;

    default:
        break;
    }

    return expr;
}

static expression_t __parser_array_generate_expression__(parser_t parse)
{
    expression_t expr;
    expression_t elem;
    list_t elements;
    long line, column;
    token_t tok;

    expr      = NULL;
    tok       = lexer_next(parse->lex);
    line      = tok->line;
    column    = tok->column;

    list_init(elements);

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RB) {
        assert((elem = __parser_rvalue_expression__(parse)) != NULL);

        list_push_back(elements, elem->link);

        if (lexer_peek(parse->lex)->value != TOKEN_VALUE_COMMA) {
            break;
        }

        tok = lexer_next(parse->lex);
    }

    __parser_expect__(parse, TOKEN_VALUE_RB, "expected ']'");

    expr = expression_new_array_generate(line, column, elements);

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_table_generate_expression__(parser_t parse)
{
    expression_t expr;
    cstring_t elemname;
    list_t members;
    long line, column;
    token_t tok;
    expression_table_pair_t pair;

    expr      = NULL;
    tok       = lexer_next(parse->lex);
    line      = tok->line;
    column    = tok->column;

    list_init(members);

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RC) {
        if (tok->value != TOKEN_VALUE_IDENTIFIER) {
            error(tok->filename, line, column, "expected identifier");
        }

        elemname = cstring_dup(tok->token);

        __parser_expect_next__(parse, TOKEN_VALUE_COLON, "expected ':'");
        
        lexer_next(parse->lex);

        pair = expression_new_table_pair(elemname, __parser_rvalue_expression__(parse));

        list_push_back(members, pair->link);

        if (lexer_peek(parse->lex)->value != TOKEN_VALUE_COMMA) {
            break;
        }

        tok = lexer_next(parse->lex);
    }

    __parser_expect__(parse, TOKEN_VALUE_RC, "expected '}'");

    expr = expression_new_table_generate(line, column, members);

    assert(expr != NULL);
    return expr;
}

static expression_t __parser_closure_definition__(parser_t parse)
{
    expression_t expr;
    long line, column;
    list_t parameters;
    list_t block;
    token_t tok;
    
    expr      = NULL;
    tok       = lexer_next(parse->lex);
    line      = tok->line;
    column    = tok->column;

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after 'function'");

    parameters = __parser_parameter_list__(parse);
    
    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    block = __parser_block__(parse);

    expr = expression_new_function(line, column, NULL, parameters, block);
    
    assert(expr != NULL);
    return expr;
}

static list_t __parser_parameter_list__(parser_t parse)
{
    expression_function_parameter_t parameter;
    list_t parameters;
    token_t tok;
    
    list_init(parameters);

    tok = lexer_peek(parse->lex);

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RP) {
        __parser_need__(parse, TOKEN_VALUE_IDENTIFIER, "expected parameter name");

        parameter = (expression_function_parameter_t) mem_alloc(sizeof(struct expression_function_parameter_s));
        parameter->name = cstring_dup(tok->token);
        
        list_push_back(parameters, parameter->link);

        tok = lexer_next(parse->lex);
        if (tok->value != TOKEN_VALUE_COMMA) {
            break;
        }

        tok = lexer_next(parse->lex);
    }

    return parameters;
}

static list_t __parser_block__(parser_t parse)
{
    list_t block;

    list_init(block);

    __parser_expect__(parse, TOKEN_VALUE_LC, "expected '{'");

    __parser_expect__(parse, TOKEN_VALUE_RC, "expected '}'");

    return block;
}

static list_t __parser_argument_list__(parser_t parse)
{
    token_t tok = lexer_next(parse->lex);
    list_t args;

    list_init(args);

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RP) {
        list_push_back(args, __parser_rvalue_expression__(parse)->link);

        if (lexer_peek(parse->lex)->value != TOKEN_VALUE_COMMA) {
            break;
        }

        tok = lexer_next(parse->lex);
    }

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    return args;
}