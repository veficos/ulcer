

#ifndef _ULCER_SOURCE_CODE_H_
#define _ULCER_SOURCE_CODE_H_

#include "config.h"

typedef enum source_code_type_e {
    SOURCE_CODE_TYPE_STRING, 
    SOURCE_CODE_TYPE_FILE,
}source_code_type_t;

typedef struct source_code_s* source_code_t;

source_code_t source_code_new(const char *s, source_code_type_t sctype);
void source_code_free(source_code_t sc);
bool source_code_iseof(source_code_t sc);
char source_code_get(source_code_t sc);
void source_code_unget(source_code_t sc, char ch);
char source_code_peek(source_code_t sc);
void source_code_reset(source_code_t sc);
const char* source_code_file_name(source_code_t sc);

#endif