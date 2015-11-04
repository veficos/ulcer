

#ifndef _NCC_TOKEN_H_
#define _NCC_TOKEN_H_

#include "config.h"
#include "cstring.h"
#include "list.h"

typedef enum token_type_e {
    TOKEN_TYPE_NIL,

    TOKEN_TYPE_INTEGER,
    TOKEN_TYPE_FLOAT,
    TOKEN_TYPE_BOOLEAN,
    TOKEN_TYPE_LITERAL,
    
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_DELIMITER,

    TOKEN_TYPE_END,
}token_type_t;

typedef enum token_value_e {
    TOKEN_VALUE_NIL, 

    TOKEN_VALUE_LP,          /* ( */
    TOKEN_VALUE_RP,          /* ) */
    TOKEN_VALUE_LC,          /* { */
    TOKEN_VALUE_RC,          /* } */
    TOKEN_VALUE_LB,          /* [ */
    TOKEN_VALUE_RB,          /* ] */
    TOKEN_VALUE_COLON,       /* : */
    TOKEN_VALUE_SEMICOLON,   /* ; */
    TOKEN_VALUE_COMMA,       /* , */
    TOKEN_VALUE_AND,         /* && */
    TOKEN_VALUE_OR,          /* || */
    TOKEN_VALUE_ASSIGN,      /* = */
    TOKEN_VALUE_EQ,          /* == */
    TOKEN_VALUE_NEQ,         /* != */
    TOKEN_VALUE_LT,          /* < */
    TOKEN_VALUE_LEQ,         /* <= */
    TOKEN_VALUE_GT,          /* > */
    TOKEN_VALUE_GEQ,         /* >= */
    TOKEN_VALUE_ADD,         /* + */
    TOKEN_VALUE_SUB,         /* - */
    TOKEN_VALUE_MUL,         /* * */
    TOKEN_VALUE_DIV,         /* / */
    TOKEN_VALUE_MOD,         /* % */
    TOKEN_VALUE_ADD_ASSIGN,  /* += */
    TOKEN_VALUE_SUB_ASSIGN,  /* -= */
    TOKEN_VALUE_MUL_ASSIGN,  /* *= */
    TOKEN_VALUE_DIV_ASSIGN,  /* /= */
    TOKEN_VALUE_MOD_ASSIGN,  /* %= */
    TOKEN_VALUE_INC,         /* ++ */
    TOKEN_VALUE_DEC,         /* -- */
    TOKEN_VALUE_NOT,         /* ! */
    TOKEN_VALUE_DOT,         /* . */

    TOKEN_VALUE_LITERAL_INT,    /* such as 1 123 */
    TOKEN_VALUE_LITERAL_LONG,   /* such as 1l */
    TOKEN_VALUE_LITERAL_FLOAT,  /* such as 1.2f */
    TOKEN_VALUE_LITERAL_DOUBLE, /* such as 1.2 */
    TOKEN_VALUE_LITERAL_CHAR,   /* such as 'a' */
    TOKEN_VALUE_LITERAL_STRING, /* such as "hello world" */

    TOKEN_VALUE_FUNCTION,
    TOKEN_VALUE_IF,
    TOKEN_VALUE_ELSE,
    TOKEN_VALUE_ELIF,
    TOKEN_VALUE_WHILE,
    TOKEN_VALUE_FOR,
    TOKEN_VALUE_FOREACH,
    TOKEN_VALUE_RETURN,
    TOKEN_VALUE_BREAK,
    TOKEN_VALUE_CONTINUE,
    TOKEN_VALUE_NULL,
    TOKEN_VALUE_TRUE,
    TOKEN_VALUE_FALSE,
    TOKEN_VALUE_CLOSURE,
    TOKEN_VALUE_GLOBAL,
    TOKEN_VALUE_TRY,
    TOKEN_VALUE_CATCH,
    TOKEN_VALUE_FINALLY,
    TOKEN_VALUE_THROW,
    TOKEN_VALUE_FINAL,

    TOKEN_VALUE_IDENTIFIER,
}token_value_t;

typedef struct token_s {
    cstring_t filename;
    cstring_t token;
    
    int numberbase;

    token_type_t  type;
    token_value_t value;

    long line;
    long column;

    list_node_t link;
}* token_t;

token_t token_new(const char *filename);
void token_free(token_t token);
void token_reset(token_t token, long line, long column, token_type_t type, token_value_t value);
token_t token_dup(token_t from);

#endif