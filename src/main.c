

#include "parser.h"
#include "native.h"
#include "lexer.h"
#include "list.h"
#include "environment.h"
#include "source_code.h"
#include "executor.h"
#include "heap.h"
#include "libsdl.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
            printf("press any key to exit");
            getchar();
            exit(-1);
        }

        sc = source_code_new(args[1], SOURCE_CODE_TYPE_FILE);
        if (sc == NULL) {
            fprintf(stderr, "ulcer: cannot open %s: No such file or directory\n", args[1]);
            exit(-1);
        }

        lex = lexer_new(sc);

        parse = parser_new(lex);

        module = parser_generate_module(parse);

        env = environment_new();

        environment_add_module(env, module);

        setup_native_module(env);

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