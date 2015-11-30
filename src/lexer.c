

#include "lexer.h"
#include "alloc.h"
#include "token.h"
#include "error.h"
#include "list.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct lexer_s {
    source_code_t   sc;
    token_t         tok;
    list_t          backups;
    token_t         last;
    long            current_line;
    long            current_column;
};

struct keyword_s {
    const char   *word;
    token_value_t tv;
};

struct keyword_s keywords[] = {
    { "require",        TOKEN_VALUE_REQUIRE     },
    { "function",       TOKEN_VALUE_FUNCTION    } ,
    { "if",             TOKEN_VALUE_IF          },
    { "else",           TOKEN_VALUE_ELSE        },
    { "elif",           TOKEN_VALUE_ELIF        },
    { "while",          TOKEN_VALUE_WHILE       },
    { "for",            TOKEN_VALUE_FOR         },
    { "switch",         TOKEN_VALUE_SWITCH      },
    { "case",           TOKEN_VALUE_CASE        },
    { "return",         TOKEN_VALUE_RETURN      },
    { "break",          TOKEN_VALUE_BREAK       },
    { "continue",       TOKEN_VALUE_CONTINUE    },
    { "null",           TOKEN_VALUE_NULL        },
    { "true",           TOKEN_VALUE_TRUE        },
    { "false",          TOKEN_VALUE_FALSE       },
};

static token_t  __lexer_next__(lexer_t lex);
static char     __lexer_next_char__(lexer_t lex);
static char     __lexer_peek_char__(lexer_t lex);
static bool     __lexer_iseof__(lexer_t lex);
static void     __lexer_recover_char__(lexer_t lex, char ch);
static void     __lexer_parse_space__(lexer_t lex);
static void     __lexer_parse_identifier__(lexer_t lex);
static void     __lexer_parse_keyword__(lexer_t lex);
static void     __lexer_parse_number__(lexer_t lex);
static void     __lexer_parse_octal__(lexer_t lex);
static void     __lexer_parse_hexadecimal__(lexer_t lex);
static void     __lexer_parse_float__(lexer_t lex);
static void     __lexer_parse_exponent__(lexer_t lex);
static void     __lexer_parse_fpostfix__(lexer_t lex);
static void     __lexer_parse_lpostfix__(lexer_t lex);
static void     __lexer_parse_operator__(lexer_t lex, token_value_t value);
static bool     __lexer_parse_div_operator__(lexer_t lex);
static void     __lexer_parse_delimiter__(lexer_t lex, token_value_t value);
static void     __lexer_parse_literal_char__(lexer_t lex);
static void     __lexer_parse_literal_string__(lexer_t lex);
static void     __lexer_parse_escape_char__(lexer_t lex);

lexer_t lexer_new(source_code_t source_code)
{
    lexer_t lex = (lexer_t) mem_alloc(sizeof(struct lexer_s));
    if (!lex) {
        return NULL;
    }

    lex->tok            = token_new(source_code_file_name(source_code));
    lex->sc             = source_code;
    lex->current_line   = 1;
    lex->current_column = 1;
    lex->last           = NULL;

    list_init(lex->backups);

    lexer_next(lex);
    return lex;
}

void lexer_free(lexer_t lex)
{
    {
        list_iter_t iter, next_iter;
        list_safe_for_each(lex->backups, iter, next_iter) {
            list_erase(lex->backups, *iter);
            token_free(list_element(iter, token_t, link));
        }
    }
    
    if (lex->last) {
        token_free(lex->last);
    }

    token_free(lex->tok);

    mem_free(lex);
}

token_t lexer_next(lexer_t lex)
{
    token_t current;

    if (!list_is_empty(lex->backups)) {
        current = list_element(list_begin(lex->backups), token_t, link);

        list_pop_front(lex->backups);
        
        if (lex->last) {
            token_free(lex->last);
        }

        lex->last = current;

        return current;
    }

    return __lexer_next__(lex);
}

token_t lexer_peek(lexer_t lex)
{
    return lex->tok;
}

void lexer_unget(lexer_t lex, token_t tok)
{
    list_push_back(lex->backups, token_dup(tok)->link);
}

token_t __lexer_next__(lexer_t lex)
{
    char ch;

reparse:
    
    __lexer_parse_space__(lex);

    ch = __lexer_peek_char__(lex);
    if (isalpha(ch) || ch == '_') {
        __lexer_parse_identifier__(lex);

    } else if (isdigit(ch)) {
        __lexer_parse_number__(lex);

    } else if (ch == '+') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_ADD);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_ADD_ASSIGN;

        } else if (__lexer_peek_char__(lex) == '+') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_INC;
        }

    } else if (ch == '-') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_SUB);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_SUB_ASSIGN;

        } else if (__lexer_peek_char__(lex) == '-') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_DEC;

        } else if (__lexer_peek_char__(lex) == '>') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_ARRAY_POP;
        }

    } else if (ch == '*') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_MUL);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_MUL_ASSIGN;
        }

    } else if (ch == '/') {
        if (!__lexer_parse_div_operator__(lex)) {
            goto reparse;
        }

        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_DIV_ASSIGN;
        }

    } else if (ch == '%') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_MOD);

        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_MOD_ASSIGN;
        }

    } else if (ch == '=') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_ASSIGN);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_EQ;
        }

    } else if (ch == '!') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_NOT);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_NEQ;
        }

    } else if (ch == '<') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_LT);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_LEQ;

        } else if (__lexer_peek_char__(lex) == '<') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_LEFT_SHIFT;

            if (__lexer_peek_char__(lex) == '=') {
                lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
                lex->tok->value = TOKEN_VALUE_LEFI_SHIFT_ASSIGN;
            }

        } else if (__lexer_peek_char__(lex) == '-') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_ARRAY_PUSH;
        }

    } else if (ch == '>') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_GT);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_GEQ;

        } else if (__lexer_peek_char__(lex) == '>') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_RIGHT_SHIFT;

            if (__lexer_peek_char__(lex) == '>') {
                lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
                lex->tok->value = TOKEN_VALUE_LOGIC_RIGHT_SHIFT;

                if (__lexer_peek_char__(lex) == '=') {
                    lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
                    lex->tok->value = TOKEN_VALUE_LOGIC_RIGHT_SHIFT_ASSIGN;
                }

            } else if (__lexer_peek_char__(lex) == '=') {
                lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
                lex->tok->value = TOKEN_VALUE_RIGHT_SHIFT_ASSIGN;
            }
        }

    } else if (ch == '&') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_BITAND);
        if (__lexer_peek_char__(lex) == '&') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_AND;

        } else if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_BITAND_ASSIGN;
        }

    } else if (ch == '|') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_BITOR);
        if (__lexer_peek_char__(lex) == '|') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_OR;

        } else if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_BITOR_ASSIGN;
        }

    } else if (ch == '^') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_XOR);
        if (__lexer_peek_char__(lex) == '=') {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            lex->tok->value = TOKEN_VALUE_XOR_ASSIGN;
        }

    } else if (ch == '~') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_CPL);

    } else if (ch == '_') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_IGNORE);

    } else if (ch == '(') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_LP);

    } else if (ch == ')') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_RP);

    } else if (ch == '[') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_LB);

    } else if (ch == ']') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_RB);

    } else if (ch == '{') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_LC);

    } else if (ch == '}') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_RC);

    } else if (ch == ':') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_COLON);

    } else if (ch == ',') {
        __lexer_parse_delimiter__(lex, TOKEN_VALUE_COMMA);

    } else if (ch == ';') {
        __lexer_parse_delimiter__(lex, TOKEN_VALUE_SEMICOLON);

    } else if (ch == '.') {
        __lexer_parse_operator__(lex, TOKEN_VALUE_DOT);

    } else if (ch == '\'') {
        __lexer_parse_literal_char__(lex);

    } else if (ch == '"') {
        __lexer_parse_literal_string__(lex);

    } else if (ch == '\0') {
        token_reset(lex->tok, lex->tok->line, lex->tok->column, TOKEN_TYPE_END, TOKEN_VALUE_NIL);

    } else if (ch == '#') {

        char ch;
        int next = 0;
        while (((ch = __lexer_next_char__(lex)) != '\n' && !__lexer_iseof__(lex)) || next--) {
            if (ch == '\\') {
                next++;
            }
        }
        goto reparse;

    } else {
        error(source_code_file_name(lex->sc),
              lex->current_line, 
              lex->current_column,
              "undeclared identifier '%c'", __lexer_peek_char__(lex));
    }

    return lex->tok;
}

static char __lexer_next_char__(lexer_t lex)
{
    char ch = source_code_get(lex->sc);
    if (ch == '\n') {
        lex->current_line++;
        lex->current_column = 1;

    } else {
        lex->current_column++;
    }

    return ch;
}

static char __lexer_peek_char__(lexer_t lex) 
{
    return source_code_peek(lex->sc);
}

static void __lexer_recover_char__(lexer_t lex, char ch)
{
    source_code_unget(lex->sc, ch);
    lex->current_column--;
}

static bool __lexer_iseof__(lexer_t lex)
{
    return source_code_iseof(lex->sc);
}

static void __lexer_parse_space__(lexer_t lex)
{
    while (isspace(__lexer_peek_char__(lex))) {
        __lexer_next_char__(lex);
    }
}

static void __lexer_parse_identifier__(lexer_t lex)
{
    char ch;

    token_reset(lex->tok, 
                lex->current_line, 
                lex->current_column,
                TOKEN_TYPE_IDENTIFIER, 
                TOKEN_VALUE_IDENTIFIER);

    do {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
        ch = __lexer_peek_char__(lex);
    } while (isalnum(ch) || ch == '_');

    __lexer_parse_keyword__(lex);
}

static void __lexer_parse_keyword__(lexer_t lex)
{
    int i;
    for (i = 0; i < sizeof(keywords) / sizeof(struct keyword_s); i++) {
        if (strcmp(keywords[i].word, lex->tok->token) == 0) {
            lex->tok->type = TOKEN_TYPE_KEYWORD;
            lex->tok->value = keywords[i].tv;
        }
    }
}

static void __lexer_parse_number__(lexer_t lex)
{
    token_reset(lex->tok,
                lex->current_line, 
                lex->current_column,
                TOKEN_TYPE_INTEGER, 
                TOKEN_VALUE_LITERAL_INT);

    if (__lexer_peek_char__(lex) == '0') {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
        
        switch (__lexer_peek_char__(lex)) {
        case '.': case 'e': case 'E':
            __lexer_parse_float__(lex);
            break;
        case 'x': case 'X':
            __lexer_parse_hexadecimal__(lex);
            break;
        default:
            __lexer_parse_octal__(lex);
            break;
        }

    } else {
        while (isdigit(__lexer_peek_char__(lex))) {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
        }

        switch (__lexer_peek_char__(lex)) {
        case '.': case 'e': case 'E':
            __lexer_parse_float__(lex);
            break;
        case 'f': case 'F':
            __lexer_parse_fpostfix__(lex);
            break;
        default:
            __lexer_parse_lpostfix__(lex);
            break;
        }
    }
}

static void __lexer_parse_octal__(lexer_t lex)
{
    while (strchr("01234567", __lexer_peek_char__(lex))) {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
    }

    switch (__lexer_peek_char__(lex)) {
    case '.': case 'e': case 'E':
        __lexer_parse_float__(lex);
        return ;
    case 'f': case 'F':
        __lexer_parse_fpostfix__(lex);
        break;
    default:
        __lexer_parse_lpostfix__(lex);
        break;
    }

    if (cstring_length(lex->tok->token) > 1) {
        lex->tok->numberbase = 8;
    }
}

static void __lexer_parse_hexadecimal__(lexer_t lex)
{
    do {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
    } while (strchr("0123456789abcdefABCDEF", __lexer_peek_char__(lex)));

    lex->tok->numberbase = 16;
}

static void __lexer_parse_float__(lexer_t lex)
{
    if (__lexer_peek_char__(lex) == '.') {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
    }

    while (isdigit(__lexer_peek_char__(lex))) {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
    }

    __lexer_parse_exponent__(lex);
}

static void __lexer_parse_exponent__(lexer_t lex)
{
    switch (__lexer_peek_char__(lex)) {
    case 'e': case 'E':
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
        
        switch (__lexer_peek_char__(lex)) {
        case '+': case '-':
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            break;
        }

        while (isdigit(__lexer_peek_char__(lex))) {
            lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
        }

        /* parse f postfix */
    default:
        __lexer_parse_fpostfix__(lex);
        break;
    }
}

static void __lexer_parse_lpostfix__(lexer_t lex)
{
    char ch = __lexer_peek_char__(lex);

    if (ch == 'l' || ch == 'L') {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
        lex->tok->value = TOKEN_VALUE_LITERAL_LONG;

    } else if (isalpha(ch)) {
        error(source_code_file_name(lex->sc), 
              lex->current_line, 
              lex->current_column,
              "invalid suffix '%c' on integer", ch);
    }
}

static void __lexer_parse_fpostfix__(lexer_t lex)
{
    char ch = __lexer_peek_char__(lex);

    if (ch == 'f' || ch == 'F') {
        lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
        lex->tok->value = TOKEN_VALUE_LITERAL_FLOAT;

        if (!isdigit(lex->tok->token[cstring_length(lex->tok->token) - 2])) {
            while (!__lexer_iseof__(lex) && !isspace(__lexer_peek_char__(lex))) {
                lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
            }

            error(source_code_file_name(lex->sc), 
                  lex->tok->line, 
                  lex->tok->column,
                  "'%s' exponent has no digits", lex->tok->token);
        }

    } else if (isalpha(ch)) {
        error(source_code_file_name(lex->sc), 
              lex->current_line, 
              lex->current_column,
              "invalid suffix '%c' on floating", ch);

    } else {
        if (!isdigit(lex->tok->token[cstring_length(lex->tok->token) - 1])) {
            error(source_code_file_name(lex->sc), 
                  lex->tok->line, 
                  lex->tok->column,
                  "'%s' exponent has no digits", lex->tok->token);
        }

        lex->tok->value = TOKEN_VALUE_LITERAL_DOUBLE;
    }

    lex->tok->type = TOKEN_TYPE_FLOAT;
}

static void __lexer_parse_operator__(lexer_t lex, token_value_t value)
{
    token_reset(lex->tok, lex->current_line, lex->current_column, TOKEN_TYPE_OPERATOR, value);
    lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
}

static bool __lexer_parse_div_operator__(lexer_t lex) 
{
    __lexer_parse_operator__(lex, TOKEN_VALUE_DIV);

    /* parse comment */
    if (__lexer_peek_char__(lex) == '/') {
        while (!__lexer_iseof__(lex) && __lexer_next_char__(lex) != '\n') ;
        return false;

    } else if (__lexer_peek_char__(lex) == '*') {
        __lexer_next_char__(lex);
        do {
            if (__lexer_iseof__(lex)) {
                error(source_code_file_name(lex->sc),
                      lex->tok->line, 
                      lex->tok->column,
                      "unterminated /* comment");
            }

            if (__lexer_next_char__(lex) == '*') {
                if (__lexer_next_char__(lex) == '/') {
                    break;
                }
            }
        } while (!__lexer_iseof__(lex));
        return false;
    }

    return true;
}

static void __lexer_parse_delimiter__(lexer_t lex, token_value_t value)
{
    token_reset(lex->tok, 
                lex->current_line, 
                lex->current_column, 
                TOKEN_TYPE_DELIMITER, 
                value);
    lex->tok->token = cstring_catch(lex->tok->token, __lexer_next_char__(lex));
}

static void __lexer_parse_escape_char__(lexer_t lex)
{
    int i;
    char ch;
    char number[4] = {0};
    char *p = &number[0];

    ch = __lexer_peek_char__(lex);
    if (strchr("01234567", ch)) {
        for (i = 0; 
             i < 3 && strchr("01234567", (ch = __lexer_peek_char__(lex)));
             i++) {
            *p++ = ch; 
            __lexer_next_char__(lex);
        }

        ch = i = strtol(number, &p, 8);
        if (i > 0xff) {
            error(source_code_file_name(lex->sc), 
                  lex->tok->line, 
                  lex->tok->column,
                  "octal escape sequence out of range");
        }

    } else if (ch == 'x') {
        __lexer_next_char__(lex);

        for (i = 0;
             i < 2 && strchr("0123456789abcdefABCDEF", (ch = __lexer_peek_char__(lex)));
             i++) {
            *p++ = ch; 
            __lexer_next_char__(lex);
        }

        ch = (char) strtol(number, &p, 16);

    } else {
        switch(ch) {
        case 'a': ch = '\a'; break;
        case 'b': ch = '\b'; break;
        case 'f': ch = '\f'; break;
        case 'n': ch = '\n'; break;
        case 'r': ch = '\r'; break;
        case 't': ch = '\t'; break;
        case 'v': ch = '\v'; break;
        case '\\': ch = '\\'; break;
        case '\'': ch = '\''; break;
        case '\"': ch = '\"'; break;
        default:
            error(source_code_file_name(lex->sc), 
                  lex->tok->line, 
                  lex->tok->column,
                  "unknown escape sequence '\\%c'", ch);
            break;
        }
        
        __lexer_next_char__(lex);
    }

    lex->tok->token = cstring_catch(lex->tok->token, ch);
}

static void __lexer_parse_literal_char__(lexer_t lex)
{
    char ch;

    token_reset(lex->tok, lex->current_line, lex->current_column,
        TOKEN_TYPE_LITERAL, TOKEN_VALUE_LITERAL_CHAR);
    
    __lexer_next_char__(lex);

    while (!__lexer_iseof__(lex)) {
        ch = __lexer_next_char__(lex);
        if (ch == '\'' || ch == '\n') {
            break;

        } else if (ch == '\\') {
            __lexer_parse_escape_char__(lex);
            
        } else {
            lex->tok->token = cstring_catch(lex->tok->token, ch);
        }
    }

    if (ch != '\'') {
        error(source_code_file_name(lex->sc),  
              lex->tok->line,
              lex->tok->column,
              "missing terminating ' character");
    }

    if (cstring_length(lex->tok->token) > 1) {
        error(source_code_file_name(lex->sc),
              lex->tok->line, 
              lex->tok->column,
              "multi-character character constant");
    }
}

static void __lexer_parse_literal_string__(lexer_t lex)
{
    char ch;

    token_reset(lex->tok, 
                lex->current_line, 
                lex->current_column,
                TOKEN_TYPE_LITERAL,
                TOKEN_VALUE_LITERAL_STRING);

    __lexer_next_char__(lex);

    while (!__lexer_iseof__(lex)) {
        ch = __lexer_next_char__(lex);
        if (ch == '"' || ch == '\n') {
            break;

        } else if (ch == '\\') {
            __lexer_parse_escape_char__(lex);

        } else {
            lex->tok->token = cstring_catch(lex->tok->token, ch);
        }
    }

    if (ch != '"') {
        error(source_code_file_name(lex->sc), 
              lex->tok->line, 
              lex->tok->column,
              "missing terminating \" character");
    }
}