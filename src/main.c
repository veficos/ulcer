

#include "config.h"
#include "alloc.h"
#include "lexer.h"
#include "token.h"
#include "array.h"
#include "stack.h"
#include "list.h"
#include "parser.h"
#include "executor.h"
#include "source_code.h"
#include "error.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

array_t print_native_function(array_t arg_stack)
{
    value_t base;
    int index;

    array_for_each(arg_stack, base, index) {
        switch (base[index].type) {
        case VALUE_TYPE_CHAR:
            printf("%c", base[index].u.char_value);
            break;

        case VALUE_TYPE_BOOL:
            base[index].u.bool_value == true ? printf("true") : printf("false");
            break;

        case VALUE_TYPE_INT:
            printf("%d", base[index].u.int_value);
            break;

        case VALUE_TYPE_LONG:
            printf("%ldl", base[index].u.long_value);
            break;

        case VALUE_TYPE_FLOAT:
            printf("%ff", base[index].u.float_value);
            break;

        case VALUE_TYPE_DOUBLE:
            printf("%lf", base[index].u.double_value);
            break;

        case VALUE_TYPE_STRING:
            printf("%s", base[index].u.object_value->u.string);
            break;

        case VALUE_TYPE_POINTER:
            printf("0x%p", base[index].u.pointer_value);
            break;

        case VALUE_TYPE_REFERENCE:
            printf("null");
            break;

        case VALUE_TYPE_NULL:
            printf("null");
            break;
        }
    }

    return NULL;
}

#undef main
int main(int argc, char **argv)
{

#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);  
#endif

    if (argc != 2) {
        printf("usage: ulcer souce.u\n");
    }

{
    source_code_t sc = source_code_new(argv[1], SOURCE_CODE_TYPE_FILE);
    lexer_t lex = lexer_new(sc);
    parser_t parse = parser_new(lex);
    module_t module = parser_generate_module(parse);
    environment_t env = environment_new();
    executor_t exec;
    
    module_add_function(module, function_new_native(cstring_new("print"), &print_native_function));
   
    environment_add_module(env, module);

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
