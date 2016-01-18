

#include <SDL.h>
#include <SDL_surface.h>

#include "libsdl.h"

static void native_sdl_init(environment_t env, unsigned int argc)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    environment_pop_value(env);
    environment_push_null(env);
}

static void native_sdl_quit(environment_t env, unsigned int argc)
{
    SDL_Quit();
    environment_pop_value(env);
    environment_push_null(env);
}

static void native_sdl_create_window(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    SDL_Window* window = NULL;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    if (argc < 6) {
        goto leave;
    }

    window = SDL_CreateWindow(values[0]->u.object_value->u.string,
        values[1]->u.int_value,
        values[2]->u.int_value,
        values[3]->u.int_value,
        values[4]->u.int_value,
        values[5]->u.int_value);

    if (window) {
        environment_push_pointer(env, (void*)window);
        return;
    }

leave:
    environment_push_null(env);
}

static void native_sdl_destroy_window(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    if (argc < 1) {
        goto leave;
    }

    SDL_DestroyWindow((SDL_Window*)values[0]->u.pointer_value);

leave:
    environment_push_null(env);
}

static void native_sdl_delay(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    SDL_Window* window = NULL;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    if (argc < 1) {
        goto leave;
    }

    SDL_Delay(values[0]->u.int_value);

leave:
    environment_push_null(env);
}


static void import_libsdl_const(environment_t env, table_t table)
{
    struct pair_s {
        char* name;
        int i;
    };

    int i;

    struct pair_s pairs[] = {
        { "SDL_WINDOWPOS_UNDEFINED",        SDL_WINDOWPOS_UNDEFINED },
        { "SDL_WINDOW_SHOWN",               SDL_WINDOW_SHOWN },
    };

    for (i = 0; i < sizeof(pairs) / sizeof(struct pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_int(env, pairs[i].i);
        table_push_pair(table, env);
    }
}

void import_libsdl_library(environment_t env)
{
    struct pair_s {
        char* name;
        native_function_pt func;
    };

    int i;
    value_t sdl_table;

    environment_push_str(env, "sdl");

    environment_push_table(env);

    sdl_table = list_element(list_rbegin(env->stack), value_t, link);

    table_push_pair(environment_get_global_table(env), env);

    import_libsdl_const(env, sdl_table->u.object_value->u.table);

    struct pair_s pairs[] = {
        { "init",               native_sdl_init },
        { "quit",               native_sdl_quit },
        { "create_window",      native_sdl_create_window },
        { "destroy_window",     native_sdl_destroy_window },
        { "delay",              native_sdl_delay },
    };

    for (i = 0; i < sizeof(pairs) / sizeof(struct pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_native_function(env, pairs[i].func);
        table_push_pair(sdl_table->u.object_value->u.table, env);
    }
}