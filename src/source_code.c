

#include "source_code.h"
#include "alloc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct source_code_s {
    union {
        FILE *fp;
        char *s;
    }u;
    const char *name;
    source_code_type_t sctype;
    long i;
};

source_code_t source_code_new(const char *s, source_code_type_t sctype)
{
    source_code_t sc = mem_alloc(sizeof(struct source_code_s));

    switch (sctype) {
    case SOURCE_CODE_TYPE_FILE:
        sc->u.fp = fopen(s, "rb");
        if (!sc->u.fp) {
            mem_free(sc);
            return NULL;
        }
        sc->name = s;
        break;

    case SOURCE_CODE_TYPE_STRING:
        sc->u.s = strdup(s);
        if (!sc->u.s) {
            mem_free(sc);
            return NULL;
        }
        sc->name = "<string>";
        break;

    default:
        return NULL;
    }

    sc->sctype = sctype;
    sc->i = 0;
    
    return sc;
}

void source_code_free(source_code_t sc)
{
    switch (sc->sctype) {
    case SOURCE_CODE_TYPE_FILE:
        fclose(sc->u.fp);
        break;

    case SOURCE_CODE_TYPE_STRING:
        mem_free(sc->u.s);
        break;
    }

    mem_free(sc);
}

bool source_code_iseof(source_code_t sc)
{
    char ch;

    switch (sc->sctype) {
    case SOURCE_CODE_TYPE_FILE:
        ch = (char) fgetc(sc->u.fp);
        if (ch == EOF) {
            return true;
        }
        ungetc(ch, sc->u.fp);
        return false;
        
    case SOURCE_CODE_TYPE_STRING:
        return sc->u.s[sc->i] == '\0';
    }

    return true;
}

char source_code_get(source_code_t sc)
{
    char ch = '\0';

    switch (sc->sctype) {
    case SOURCE_CODE_TYPE_FILE:
        sc->i++;
        ch = (char) fgetc(sc->u.fp);
        break;

    case SOURCE_CODE_TYPE_STRING:
        ch = sc->u.s[sc->i++];
        break;
    }

    return ch;
}

void source_code_unget(source_code_t sc, char ch)
{
    switch (sc->sctype) {
    case SOURCE_CODE_TYPE_FILE:
        ungetc(ch, sc->u.fp);
        break;
    case SOURCE_CODE_TYPE_STRING: 
        break;
    }

    if (sc->i > 0) {
        sc->i--;
    }
}

char source_code_peek(source_code_t sc)
{
    char ch;
    switch (sc->sctype) {
    case SOURCE_CODE_TYPE_FILE:
        ch = (char) fgetc(sc->u.fp);
        if (ch == EOF) {
            return '\0';
        }
        ungetc(ch, sc->u.fp);
        return ch;

    case SOURCE_CODE_TYPE_STRING:
        return sc->u.s[sc->i];
    }

    return '\0';
}

void source_code_reset(source_code_t sc)
{
    switch (sc->sctype) {
    case SOURCE_CODE_TYPE_FILE:
        break;

    case SOURCE_CODE_TYPE_STRING:
        
        fseek(sc->u.fp, 0, SEEK_SET);
        break;
    }

    sc->i = 0;
}

const char* source_code_file_name(source_code_t sc)
{
    return sc->name;
}
