

#include "source_code.h"
#include "parser.h"
#include "lexer.h"
#include "list.h"
#include "environment.h"
#include "executor.h"
#include "heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void print_value(value_t value)
{
    switch (value->type) {
    case VALUE_TYPE_CHAR:
        printf("%c", value->u.char_value);
        break;

    case VALUE_TYPE_BOOL:
        value->u.bool_value == true ? printf("true") : printf("false");
        break;

    case VALUE_TYPE_INT:
        printf("%d", value->u.int_value);
        break;

    case VALUE_TYPE_LONG:
        printf("%ldl", value->u.long_value);
        break;

    case VALUE_TYPE_FLOAT:
        printf("%ff", value->u.float_value);
        break;

    case VALUE_TYPE_DOUBLE:
        printf("%lf", value->u.double_value);
        break;

    case VALUE_TYPE_STRING:
        printf("%s", value->u.object_value->u.string);
        break;

    case VALUE_TYPE_NULL:
        printf("null");
        break;

    case VALUE_TYPE_ARRAY:
        {
            int index;
            int last;
            value_t* base;

            last = array_length(value->u.object_value->u.array) - 1;

            printf("[");
            array_for_each(value->u.object_value->u.array, base, index) {
                print_value(base[index]);
                if (last != index) {
                    printf(", ");
                }
            }
            printf("]");
        }
        break;

    case VALUE_TYPE_TABLE:
        {
            hash_table_iter_t hiter;
            table_pair_t pair;
            int index;
            int last;

            index = 0;
            last = hash_table_size(value->u.object_value->u.table->table) - 1;

            printf("{");
            hash_table_for_each(value->u.object_value->u.table->table, hiter) {
                pair = hash_table_iter_element(hiter, table_pair_t, link);

                print_value(pair->key);

                printf(":");

                print_value(pair->value);

                if (last != index++) {
                    printf(", ");
                }
            }
            printf("}");
        }
        break;

    case VALUE_TYPE_FUNCTION:
        printf("function");
        break;

    default:
        assert(false);
        break;
    }
}

static void native_print(environment_t env, unsigned int argc)
{
    value_t value;
    list_iter_t iter;
    int i = 0;

    list_for_each(env->stack, iter) {        
        value = list_element(iter, value_t, link);
        print_value(value);
    }
    
    environment_clear_stack(env);

    environment_push_null(env);
}

static void setup_stdlib(environment_t env)
{
    environment_push_str(env, "print");
    
    environment_push_native_function(env, native_print);

    table_push_pair(environment_get_global_table(env), env);

    environment_push_str(env, "shit");

    environment_push_native_function(env, native_print);

    table_push_pair(environment_get_global_table(env), env);
}

int main(int argc, char** args)
{
    {
#ifdef WIN32
        _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);  
#endif
    }

    {
        source_code_t sc;
        lexer_t       lex;
        parser_t      parse;
        module_t      module;
        environment_t env;
        executor_t    executor;

        if (argc < 2) {
            printf("usage: ulcer souce_code.ul\n");
            exit(-1);
        }

        sc = source_code_new(args[1], SOURCE_CODE_TYPE_FILE);
        lex = lexer_new(sc);
        parse = parser_new(lex);

        module = parser_generate_module(parse);

        env = environment_new();

        environment_add_module(env, module);

        setup_stdlib(env);

        executor_run((executor = executor_new(env)));

        assert(list_is_empty(env->stack));

        executor_free(executor);
        environment_free(env);
        parser_free(parse);
        lexer_free(lex);
        source_code_free(sc);
    }

    return 0;
}