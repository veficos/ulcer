

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
static statement_t  __parser_statement__(parser_t parse);
static statement_t  __parser_if_statement__(parser_t parse);
static list_t       __parser_elifs_statement__(parser_t parse);
static list_t       __parser_else_statement__(parser_t parse);
static statement_t  __parser_switch_statement__(parser_t parse);
static statement_t  __parser_while_statement__(parser_t parse);
static statement_t  __parser_for_statement__(parser_t parse);
static expression_t __parser_expression__(parser_t parse);
static bool         __parser_check_expression__(cstring_t filename, expression_t expr);
static void         __parser_without_function_expression__(cstring_t filename, expression_t expr);
static bool         __parser_check_lvalue_expression__(expression_t expr);
static expression_t __parser_assign_expression__(parser_t parse);
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
static expression_t __parser_generate_expression__(parser_t parse); 
static expression_t __parser_array_generate_expression__(parser_t parse);
static expression_t __parser_table_generate_expression__(parser_t parse);
static expression_t __parser_function_definition__(parser_t parse);
static list_t       __parser_parameter_list__(parser_t parse);
static list_t       __parser_argument_list__(parser_t parse);
static list_t       __parser_block__(parser_t parse);

parser_t parser_new(lexer_t lex)
{
    parser_t parse = (parser_t) mem_alloc(sizeof(struct parser_s));
    if (!parse) {
        return NULL;
    }

    parse->lex    = lex;
    parse->module = module_new();
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
    statement_t stmt;
    switch (lexer_peek(parse->lex)->value) {
    case TOKEN_VALUE_REQUIRE:
        module_add_statment(parse->module, __parser_require_statement__(parse));
        break;
    
    case TOKEN_VALUE_SEMICOLON:
        lexer_next(parse->lex);
        break;

    default:
        stmt = __parser_statement__(parse);
        if (stmt->type == STATEMENT_TYPE_EXPRESSION && 
            stmt->u.expr->type == EXPRESSION_TYPE_FUNCTION) {
            module_add_function(parse->module, stmt);
        } else {
            module_add_statment(parse->module, stmt);
        }
        break;
    }
}

static statement_t __parser_require_statement__(parser_t parse)
{
    long        line;
    long        column;
    token_t     tok;
    statement_t stmt;

    stmt   = NULL;
    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    __parser_expect_next__(parse, TOKEN_VALUE_LITERAL_STRING, "require expects \"FILENAME\"");

    stmt = statement_new_require(line, column, cstring_dup(lexer_peek(parse->lex)->token));

    lexer_next(parse->lex);

    assert(stmt != NULL);
    return stmt;
}

static statement_t __parser_statement__(parser_t parse)
{
    long         line; 
    long         column;
    token_t      tok;
    statement_t  stmt;
    expression_t expr;
 
    stmt    = NULL;
    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    switch (tok->value) {
    case TOKEN_VALUE_IF:
        stmt = __parser_if_statement__(parse);
        break;

    case TOKEN_VALUE_WHILE:
        stmt = __parser_while_statement__(parse);
        break;

    case TOKEN_VALUE_SWITCH:
        stmt = __parser_switch_statement__(parse);
        break;

    case TOKEN_VALUE_FOR:
        stmt = __parser_for_statement__(parse);
        break;

    case TOKEN_VALUE_RETURN:
        if (lexer_next(parse->lex)->value != TOKEN_VALUE_SEMICOLON) {
            __parser_check_expression__(tok->filename, (expr = __parser_expression__(parse)));
            stmt = statement_new_return(line, column, expr);
        } else {
            stmt = statement_new_return(line, column, NULL);
        }
        break;

    case TOKEN_VALUE_BREAK:
        __parser_expect_next__(parse, TOKEN_VALUE_SEMICOLON, "expected ';'");
        lexer_next(parse->lex);
        stmt = statement_new_break(line, column);
        break;

    case TOKEN_VALUE_CONTINUE:
        lexer_next(parse->lex);
        stmt = statement_new_continue(line, column);
        break;

    default:
        stmt = statement_new_expression(line, column, __parser_expression__(parse));
        if (stmt->u.expr->type != EXPRESSION_TYPE_FUNCTION) {
            __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, "expected ';'");
        }
        break;
    }

    return stmt;
}

static statement_t __parser_if_statement__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    statement_t  if_stmt;
    expression_t if_condition;
    list_t       if_block;
    list_t       elifs;
    list_t       else_block;

    if_stmt      = NULL;
    if_condition = NULL;
    
    list_init(if_block);
    list_init(elifs);
    list_init(else_block);

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after if");

    if_condition = __parser_expression__(parse);

    __parser_without_function_expression__(tok->filename, if_condition);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    if_block = __parser_block__(parse);

    switch (lexer_peek(parse->lex)->value) {
    case TOKEN_VALUE_ELIF:
        elifs = __parser_elifs_statement__(parse);
       
    default:
        if (lexer_peek(parse->lex)->value == TOKEN_VALUE_ELSE) {
            else_block = __parser_else_statement__(parse);
        }
        break;
    }

    assert((if_stmt = statement_new_if(line, column, if_condition, if_block, elifs, else_block)) != NULL);
    return if_stmt;
}

static list_t __parser_elifs_statement__(parser_t parse)
{
    token_t          tok;
    list_t           elifs;
    expression_t     condition;
    statement_elif_t elif_block;
    
    list_init(elifs);

    tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && tok->value == TOKEN_VALUE_ELIF) {
        tok = lexer_next(parse->lex);

        __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after elif");

        condition = __parser_expression__(parse);
        
        __parser_without_function_expression__(tok->filename, condition);

        __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

        elif_block = statement_new_elif(condition, __parser_block__(parse));

        list_push_back(elifs, elif_block->link);
    }

    return elifs;
}

static list_t __parser_else_statement__(parser_t parse)
{
    lexer_next(parse->lex);

    return __parser_block__(parse);
}

static statement_t __parser_switch_statement__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    statement_t  switch_stmt;
    expression_t switch_expr;
    expression_t case_expr;
    list_t       case_block;
    list_t       cases;
    list_t       default_block;
    bool         has_default;

    list_init(cases);
    list_init(default_block);

    switch_stmt = NULL;
    has_default = false;
    tok         = lexer_peek(parse->lex);
    line        = tok->line;
    column      = tok->column;

    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after switch");

    switch_expr = __parser_expression__(parse);

    __parser_without_function_expression__(tok->filename, switch_expr);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    __parser_expect__(parse, TOKEN_VALUE_LC, "expected '{'");

    tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RC) {
        if (tok->value != TOKEN_VALUE_CASE && tok->value != TOKEN_VALUE_DEFAULT) {
            error(tok->filename, tok->line, tok->column, "expected 'case expr:' or 'default:'");
        }

        if (tok->value == TOKEN_VALUE_CASE) {
            lexer_next(parse->lex);

            case_expr = __parser_expression__(parse);

            __parser_without_function_expression__(tok->filename, case_expr);

            __parser_expect__(parse, TOKEN_VALUE_COLON, "expected ':'");

            case_block = __parser_block__(parse);

            list_push_back(cases, statement_new_switch_case(case_expr, case_block)->link);

        } else if (!has_default) {
            lexer_next(parse->lex);
            
            __parser_expect__(parse, TOKEN_VALUE_COLON, "expected ':'");
            
            default_block = __parser_block__(parse);
            
            has_default = true;

        } else {
            error(tok->filename, tok->line, tok->column, "multiple default labels in one switch");
        }
    }

    __parser_expect__(parse, TOKEN_VALUE_RC, "expected '}'");

    assert((switch_stmt = statement_new_switch(line, column, switch_expr, cases, default_block)) != NULL);
    return switch_stmt;
}

static statement_t __parser_while_statement__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    statement_t  while_stmt;
    expression_t condition;
    list_t       block;
    
    while_stmt = NULL;
    tok        = lexer_peek(parse->lex);
    line       = tok->line;
    column     = tok->column;
    
    lexer_next(parse->lex);

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after while");

    condition = __parser_expression__(parse);

    __parser_without_function_expression__(tok->filename, condition);

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    block = __parser_block__(parse);

    assert((while_stmt = statement_new_while(line, column, condition, block)) != NULL);
    return while_stmt;
}

static statement_t __parser_for_statement__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    statement_t  for_stmt;
    expression_t init;
    expression_t condition;
    expression_t post;
    list_t       block;

    for_stmt   = NULL;
    init       = NULL;
    condition  = NULL;
    post       = NULL;
    tok        = lexer_peek(parse->lex);
    line       = tok->line;
    column     = tok->column;

    list_init(block);

    lexer_next(parse->lex);
   
    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' at for");

    if (lexer_peek(parse->lex)->value != TOKEN_VALUE_SEMICOLON) {
        init = __parser_expression__(parse);
        __parser_without_function_expression__(tok->filename, init);
    }

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, "expected ';'");

    if (lexer_peek(parse->lex)->value != TOKEN_VALUE_SEMICOLON) {
        condition = __parser_expression__(parse);
        __parser_without_function_expression__(tok->filename, condition);
    }

    __parser_expect__(parse, TOKEN_VALUE_SEMICOLON, "expected ';'");

    if (lexer_peek(parse->lex)->value != TOKEN_VALUE_RP) {
        post = __parser_expression__(parse);
        __parser_without_function_expression__(tok->filename, post);
    }

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    block = __parser_block__(parse);

    assert((for_stmt = statement_new_for(line, column, init, condition, post, block)) != NULL);
    return for_stmt;
}

static expression_t __parser_expression__(parser_t parse)
{
    return __parser_assign_expression__(parse);
}

static bool __parser_check_expression__(cstring_t filename, expression_t expr)
{
    if (expr->type == EXPRESSION_TYPE_FUNCTION) {
        if (!cstring_is_empty(expr->u.function_expr->name)) {
            error(filename, expr->line, expr->column, "expected anonymous function expression");
        }
        /* function expression */
        return true;
    }
    /* normal expression */
    return false;
}

static void __parser_without_function_expression__(cstring_t filename, expression_t expr)
{
    if (__parser_check_expression__(filename, expr)) {
        error(filename, expr->line, expr->column, "unexpected expression");
    }
}

static bool __parser_check_lvalue_expression__(expression_t expr)
{
    if (expr->type != EXPRESSION_TYPE_IDENTIFIER &&
        expr->type != EXPRESSION_TYPE_INDEX      && 
        expr->type != EXPRESSION_TYPE_TABLE_DOT_MEMBER) {
        return false;
    }
    return true;
}

static expression_t __parser_assign_expression__(parser_t parse)
{
    long              line;
    long              column;
    token_t           tok;
    expression_t      lexpr;
    expression_t      rexpr;
    expression_type_t expr_type;

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    lexpr  = __parser_logical_or_expression__(parse);

    switch (lexer_peek(parse->lex)->value) {
    case TOKEN_VALUE_ASSIGN:
        expr_type = EXPRESSION_TYPE_ASSIGN;
        break;

    case TOKEN_VALUE_ADD_ASSIGN:
        expr_type = EXPRESSION_TYPE_ADD_ASSIGN;
        break;

    case TOKEN_VALUE_SUB_ASSIGN:
        expr_type = EXPRESSION_TYPE_SUB_ASSIGN;
        break;

    case TOKEN_VALUE_MUL_ASSIGN:
        expr_type = EXPRESSION_TYPE_MUL_ASSIGN;
        break;

    case TOKEN_VALUE_DIV_ASSIGN:
        expr_type = EXPRESSION_TYPE_DIV_ASSIGN;
        break;

    case TOKEN_VALUE_MOD_ASSIGN:
        expr_type = EXPRESSION_TYPE_MOD_ASSIGN;
        break;

    case TOKEN_VALUE_BITAND_ASSIGN:
        expr_type = EXPRESSION_TYPE_BITAND_ASSIGN;
        break;

    case TOKEN_VALUE_BITOR_ASSIGN:
        expr_type = EXPRESSION_TYPE_BITOR_ASSIGN;
        break;

    case TOKEN_VALUE_XOR_ASSIGN:
        expr_type = EXPRESSION_TYPE_XOR_ASSIGN;
        break;

    case TOKEN_VALUE_LEFI_SHIFT_ASSIGN:
        expr_type = EXPRESSION_TYPE_LEFT_SHIFT_ASSIGN;
        break;

    case TOKEN_VALUE_RIGHT_SHIFT_ASSIGN:
        expr_type = EXPRESSION_TYPE_RIGHT_SHIFT_ASSIGN;
        break;

    case TOKEN_VALUE_LOGIC_RIGHT_SHIFT_ASSIGN:
        expr_type = EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT_ASSIGN;
        break;

    default:
        goto leave;
        break;
    }

    lexer_next(parse->lex);

    if (!__parser_check_lvalue_expression__(lexpr))  {
        error(tok->filename, line, column, "expected lvalue expression");
    }

    __parser_check_expression__(tok->filename, (rexpr = __parser_expression__(parse)));

    lexpr = expression_new_assign(line, column, expr_type, lexpr, rexpr);

leave:
    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_logical_or_expression__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    expression_t lexpr;
    expression_t rexpr;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    lexpr   = __parser_logical_and_expression__(parse);

    while (tok->value == TOKEN_VALUE_OR) {
        __parser_without_function_expression__(tok->filename, lexpr);

        lexer_next(parse->lex);

        __parser_without_function_expression__(tok->filename, (rexpr =  __parser_logical_and_expression__(parse)));

        lexpr   = expression_new_binary(line, column, EXPRESSION_TYPE_OR, lexpr, rexpr);
        tok     = lexer_peek(parse->lex);
        line    = tok->line;
        column  = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_logical_and_expression__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    expression_t lexpr;
    expression_t rexpr;

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    lexpr  = __parser_equality_expression__(parse);

    while (tok->value == TOKEN_VALUE_AND) {
        __parser_without_function_expression__(tok->filename, lexpr);

        lexer_next(parse->lex);

        __parser_without_function_expression__(tok->filename, (rexpr = __parser_equality_expression__(parse)));

        lexpr  = expression_new_binary(line, column, EXPRESSION_TYPE_AND, lexpr, rexpr);
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_equality_expression__(parser_t parse)
{
    long              line;
    long              column;
    token_t           tok;
    token_value_t     tv;
    expression_t      lexpr;
    expression_t      rexpr;
    expression_type_t expr_type;

    lexpr   = __parser_relational_expression__(parse);

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    while (tok->value == TOKEN_VALUE_EQ || 
           tok->value == TOKEN_VALUE_NEQ) {
        __parser_without_function_expression__(tok->filename, lexpr);

        tv  = tok->value;

        lexer_next(parse->lex);

        switch (tv) {
        case TOKEN_VALUE_EQ:
            expr_type = EXPRESSION_TYPE_EQ;
            break;

        case TOKEN_VALUE_NEQ:
            expr_type = EXPRESSION_TYPE_NEQ;
            break;

        default:
            assert(false);
            break;
        }

        __parser_without_function_expression__(tok->filename, (rexpr = __parser_relational_expression__(parse)));
        
        lexpr  = expression_new_binary(line, column, expr_type, lexpr, rexpr);
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_relational_expression__(parser_t parse)
{
    long              line;
    long              column;
    token_t           tok;
    token_value_t     tv;
    expression_t      lexpr;
    expression_t      rexpr;
    expression_type_t expr_type;

    tok    = lexer_peek(parse->lex);
    line   = tok->line;
    column = tok->column;

    lexpr   = __parser_additive_expression__(parse);

    while (tok->value == TOKEN_VALUE_GT  || 
           tok->value == TOKEN_VALUE_GEQ || 
           tok->value == TOKEN_VALUE_LT  || 
           tok->value == TOKEN_VALUE_LEQ) {
       __parser_without_function_expression__(tok->filename, lexpr);

        tv = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_GT:
            expr_type = EXPRESSION_TYPE_GT;
            break;

        case TOKEN_VALUE_GEQ:
            expr_type = EXPRESSION_TYPE_GEQ;
            break;

        case TOKEN_VALUE_LT:
            expr_type = EXPRESSION_TYPE_LT;
            break;

        case TOKEN_VALUE_LEQ:
            expr_type = EXPRESSION_TYPE_LEQ;
            break;

        default:
            assert(false);
            break;
        }

        __parser_without_function_expression__(tok->filename, (rexpr = __parser_additive_expression__(parse)));

        lexpr = expression_new_binary(line, column, expr_type, lexpr, rexpr);

        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_additive_expression__(parser_t parse)
{
    long              line;
    long              column;
    token_t           tok;
    token_value_t     tv;
    expression_t      lexpr;
    expression_t      rexpr;
    expression_type_t expr_type;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    lexpr   = __parser_multiplicative_expression__(parse);

    while (tok->value == TOKEN_VALUE_ADD || 
           tok->value == TOKEN_VALUE_SUB) {
        __parser_without_function_expression__(tok->filename, lexpr);

        tv = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_ADD:
            expr_type = EXPRESSION_TYPE_ADD;
            break;

        case TOKEN_VALUE_SUB:
            expr_type = EXPRESSION_TYPE_SUB;
            break;

        default:
            assert(false);
            break;
        }

        __parser_without_function_expression__(tok->filename, (rexpr = __parser_multiplicative_expression__(parse)));

        lexpr  = expression_new_binary(line, column, expr_type, lexpr, rexpr);
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_multiplicative_expression__(parser_t parse)
{
    long              line;
    long              column;
    token_t           tok;
    token_value_t     tv;
    expression_t      lexpr;
    expression_t      rexpr;
    expression_type_t expr_type;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    lexpr   = __parser_bitop_expression__(parse);

    while (tok->value == TOKEN_VALUE_MUL ||
           tok->value == TOKEN_VALUE_DIV || 
           tok->value == TOKEN_VALUE_MOD) {
        __parser_without_function_expression__(tok->filename, lexpr);

        tv = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_MUL:
            expr_type = EXPRESSION_TYPE_MUL;
            break;

        case TOKEN_VALUE_DIV:
            expr_type = EXPRESSION_TYPE_DIV;
            break;

        case TOKEN_VALUE_MOD:
            expr_type = EXPRESSION_TYPE_MOD;
            break;

        default:
            assert(false);
            break;
        }

        __parser_without_function_expression__(tok->filename, (rexpr = __parser_bitop_expression__(parse)));

        lexpr  = expression_new_binary(line, column, expr_type, lexpr, rexpr);
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_bitop_expression__(parser_t parse)
{
    long              line; 
    long              column;
    token_t           tok;
    token_value_t     tv;
    expression_t      lexpr;
    expression_t      rexpr;
    expression_type_t expr_type;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    lexpr    = __parser_shift_bitop_expression__(parse);

    while (tok->value == TOKEN_VALUE_BITAND ||
           tok->value == TOKEN_VALUE_BITOR  || 
           tok->value == TOKEN_VALUE_XOR) {
        __parser_without_function_expression__(tok->filename, lexpr);

        tv = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_BITAND:
            expr_type = EXPRESSION_TYPE_BITAND;
            break;

        case TOKEN_VALUE_BITOR:
            expr_type = EXPRESSION_TYPE_BITOR;
            break;

        case TOKEN_VALUE_XOR:
            expr_type = EXPRESSION_TYPE_XOR;
            break;

        default:
            assert(false);
            break;
        }

        __parser_without_function_expression__(tok->filename, (rexpr = __parser_shift_bitop_expression__(parse)));
        
        lexpr  = expression_new_binary(line, column, expr_type, lexpr, rexpr);
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_shift_bitop_expression__(parser_t parse)
{
    long              line;
    long              column;
    token_t           tok;
    token_value_t     tv;
    expression_t      lexpr;
    expression_t      rexpr;
    expression_type_t expr_type;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    lexpr   = __parser_unary_expression__(parse);

    while (tok->value == TOKEN_VALUE_LEFT_SHIFT  ||
           tok->value == TOKEN_VALUE_RIGHT_SHIFT || 
           tok->value == TOKEN_VALUE_LOGIC_RIGHT_SHIFT_ASSIGN) {
        __parser_without_function_expression__(tok->filename, lexpr);

        tv = tok->value;

        lexer_next(parse->lex);
        switch (tv) {
        case TOKEN_VALUE_LEFT_SHIFT:
            expr_type = EXPRESSION_TYPE_LEFT_SHIFT;
            break;

        case TOKEN_VALUE_RIGHT_SHIFT:
            expr_type = EXPRESSION_TYPE_RIGHT_SHIFT;
            break;

        case TOKEN_VALUE_LOGIC_RIGHT_SHIFT_ASSIGN:
            expr_type = EXPRESSION_TYPE_LOGIC_RIGHT_SHIFT;
            break;

        default:
            assert(false);
            break;
        }

        __parser_without_function_expression__(tok->filename, (rexpr = __parser_unary_expression__(parse)));

        lexpr  = expression_new_binary(line, column, expr_type, lexpr, rexpr);
        tok    = lexer_peek(parse->lex);
        line   = tok->line;
        column = tok->column;
    }

    assert(lexpr != NULL);
    return lexpr;
}

static expression_t __parser_unary_expression__(parser_t parse)
{
    long              line;
    long              column;
    token_t           tok;
    expression_type_t expr_type;
    expression_t      expr;
    expression_t      rexpr;

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

    __parser_without_function_expression__(tok->filename, (rexpr = __parser_unary_expression__(parse)));

    expr = expression_new_unary(line, column, expr_type, rexpr);

done:
    assert(expr != NULL);
    return expr;
}

static expression_t __parser_postfix_expression__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    expression_t expr;
    expression_t rexpr;

    tok     = lexer_peek(parse->lex);
    line    = tok->line;
    column  = tok->column;

    expr = __parser_primary_expression__(parse);

    tok  = lexer_peek(parse->lex);
    while (tok->value == TOKEN_VALUE_LB         ||
           tok->value == TOKEN_VALUE_ARRAY_PUSH ||
           tok->value == TOKEN_VALUE_DOT        ||
           tok->value == TOKEN_VALUE_LP         ||
           tok->value == TOKEN_VALUE_INC        ||
           tok->value == TOKEN_VALUE_DEC) {

        __parser_check_expression__(tok->filename, expr);

        switch (tok->value) {
        case TOKEN_VALUE_LB:
            /* index expression */
            lexer_next(parse->lex);

            __parser_without_function_expression__(tok->filename, (rexpr = __parser_expression__(parse)));

            expr = expression_new_index(line, column, expr, rexpr);

            __parser_expect__(parse, TOKEN_VALUE_RB, "expected ']'");
            break;

        case TOKEN_VALUE_ARRAY_PUSH:
            /* array push expression */
            lexer_next(parse->lex);

            __parser_check_expression__(tok->filename, (rexpr = __parser_expression__(parse)));

            expr = expression_new_array_push(line, column, expr, rexpr);
            break;

        case TOKEN_VALUE_ARRAY_POP:
            lexer_next(parse->lex);

            rexpr = __parser_expression__(parse);

            if (!__parser_check_lvalue_expression__(rexpr)) {
                error(tok->filename, line, column, "expected lvalue expression");
            }
            
            expr = expression_new_array_pop(line, column, expr, rexpr);
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
            if (!__parser_check_lvalue_expression__(expr)) {
                error(tok->filename, line, column, "expected lvalue expression");
            }
            expr = expression_new_incdec(line, column, EXPRESSION_TYPE_INC, expr);
            break;

        case TOKEN_VALUE_DEC:
            lexer_next(parse->lex);
            if (!__parser_check_lvalue_expression__(expr)) {
                error(tok->filename, line, column, "expected lvalue expression");
            }
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
    long         line;
    long         column;
    token_t      tok;
    expression_t expr;

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
        expr = __parser_function_definition__(parse);
        break;

    case TOKEN_VALUE_LP:
        lexer_next(parse->lex);
        expr = __parser_expression__(parse);
        __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");
        break;

    case TOKEN_VALUE_LB:
        expr = __parser_array_generate_expression__(parse);
        break;

    case TOKEN_VALUE_LC:
        expr = __parser_table_generate_expression__(parse);
        break;

    default:
        error(tok->filename, line, column, "expected expression");
        break;
    }

    return expr;
}

static expression_t __parser_array_generate_expression__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    expression_t expr;
    expression_t elem;
    list_t       elements;

    expr      = NULL;
    line      = lexer_peek(parse->lex)->line;
    column    = lexer_peek(parse->lex)->column;
    tok       = lexer_next(parse->lex);

    list_init(elements);

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RB) {
        __parser_check_expression__(tok->filename, (elem = __parser_expression__(parse)));

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
    long                    line;
    long                    column;
    token_t                 tok;
    expression_t            expr;
    list_t                  members;
    expression_table_pair_t pair;
    expression_t            value_expr;
    expression_t            name_expr;

    expr      = NULL;
    line      = lexer_peek(parse->lex)->line;
    column    = lexer_peek(parse->lex)->column;
    tok       = lexer_next(parse->lex);

    list_init(members);

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RC) {
        __parser_check_expression__(tok->filename, (name_expr = __parser_expression__(parse)));

        __parser_expect__(parse, TOKEN_VALUE_COLON, "expected ':'");

        __parser_check_expression__(tok->filename, (value_expr = __parser_expression__(parse)));

        pair = expression_new_table_pair(name_expr, value_expr);

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

static expression_t __parser_function_definition__(parser_t parse)
{
    long         line;
    long         column;
    token_t      tok;
    expression_t expr;
    cstring_t    funcname;
    list_t       parameters;
    list_t       block;
    
    expr   = NULL;
    line   = lexer_peek(parse->lex)->line;
    column = lexer_peek(parse->lex)->column;
    tok    = lexer_next(parse->lex);

    if (tok->value == TOKEN_VALUE_IDENTIFIER) {
        funcname = cstring_dup(tok->token);
        lexer_next(parse->lex);
    } else {
        funcname = cstring_new("");
    }

    __parser_expect__(parse, TOKEN_VALUE_LP, "expected '(' after 'function'");

    parameters = __parser_parameter_list__(parse);
    
    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    block = __parser_block__(parse);

    expr = expression_new_function(line, column, funcname, parameters, block);
    
    assert(expr != NULL);
    return expr;
}

static list_t __parser_parameter_list__(parser_t parse)
{
    expression_function_parameter_t parameter;
    list_t                          parameters;
    token_t                         tok;
    
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

static list_t __parser_argument_list__(parser_t parse)
{
    token_t      tok;
    list_t       args;
    expression_t expr;

    tok = lexer_next(parse->lex);

    list_init(args);

    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RP) {
        __parser_check_expression__(tok->filename, (expr = __parser_expression__(parse)));

        list_push_back(args, expr->link);

        if (lexer_peek(parse->lex)->value != TOKEN_VALUE_COMMA) {
            break;
        }

        tok = lexer_next(parse->lex);
    }

    __parser_expect__(parse, TOKEN_VALUE_RP, "expected ')'");

    return args;
}

static list_t __parser_block__(parser_t parse)
{
    statement_t stmt;
    list_t      block;
    token_t     tok;

    list_init(block);

    __parser_expect__(parse, TOKEN_VALUE_LC, "expected '{'");

    tok = lexer_peek(parse->lex);
    while (tok->type != TOKEN_TYPE_END && tok->value != TOKEN_VALUE_RC) {
        if (tok->value == TOKEN_VALUE_SEMICOLON) {
            tok = lexer_next(parse->lex);

        } else {
            stmt = __parser_statement__(parse);
            if (stmt->type == STATEMENT_TYPE_EXPRESSION && 
                stmt->u.expr->type == EXPRESSION_TYPE_FUNCTION) {
                __parser_check_expression__(tok->filename, stmt->u.expr);
            }

            list_push_back(block, stmt->link);
        }
    }

    __parser_expect__(parse, TOKEN_VALUE_RC, "expected '}'");

    return block;
}