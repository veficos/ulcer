

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
    environment_push_str(env, "SDL_WINDOWPOS_UNDEFINED");
    environment_push_int(env, SDL_WINDOWPOS_UNDEFINED);
    table_push_pair(table, env);

    environment_push_str(env, "SDL_WINDOW_SHOWN");
    environment_push_int(env, SDL_WINDOW_SHOWN);
    table_push_pair(table, env);
}

void import_libsdl_library(environment_t env)
{
    value_t sdl_table;

    environment_push_str(env, "sdl");

    environment_push_table(env);

    sdl_table = list_element(list_rbegin(env->stack), value_t, link);

    table_push_pair(environment_get_global_table(env), env);

    import_libsdl_const(env, sdl_table->u.object_value->u.table);

    environment_push_str(env, "init");
    environment_push_native_function(env, native_sdl_init);
    table_push_pair(sdl_table->u.object_value->u.table, env);

    environment_push_str(env, "quit");
    environment_push_native_function(env, native_sdl_quit);
    table_push_pair(sdl_table->u.object_value->u.table, env);

    environment_push_str(env, "create_window");
    environment_push_native_function(env, native_sdl_create_window);
    table_push_pair(sdl_table->u.object_value->u.table, env);

    environment_push_str(env, "delay");
    environment_push_native_function(env, native_sdl_delay);
    table_push_pair(sdl_table->u.object_value->u.table, env);
}