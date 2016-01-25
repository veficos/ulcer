

#include "config.h"
#include "alloc.h"
#include "native.h"

#undef bool
#undef true
#undef false

#ifdef USE_LIBSDL

#include <SDL.h>
#include <SDL_surface.h>

#pragma comment (lib, "SDL2.lib")
#pragma comment (lib, "SDL2main.lib")

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

    if (argc < 6) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    window = SDL_CreateWindow(native_check_string_value(values[0]),
                              native_check_int_value(values[1]),
                              native_check_int_value(values[2]),
                              native_check_int_value(values[3]),
                              native_check_int_value(values[4]),
                              native_check_int_value(values[5]));

    if (window) {
        environment_push_pointer(env, (void*)window);
    } else {
        environment_push_null(env);
    }

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_sdl_destroy_window(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        goto leave;
    }

    SDL_DestroyWindow((SDL_Window*)native_check_pointer_value(values[0]));

leave:
    environment_pop_value(env);

    environment_push_null(env);
}

static void native_sdl_create_renderer(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    SDL_Renderer* renderer = NULL;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 3) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    renderer = SDL_CreateRenderer((SDL_Window*)native_check_pointer_value(values[0]),
                                   native_check_int_value(values[1]),
                                   native_check_int_value(values[2]));

    if (renderer) {
        environment_push_pointer(env, (void*)renderer);
    } else {
        environment_push_null(env);
    }
    
    environment_xchg_stack(env);
    
    environment_pop_value(env);
}

static void native_sdl_destroy_renderer(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        goto leave;
    }

    SDL_DestroyRenderer((SDL_Renderer*)native_check_pointer_value(values[0]));

leave:
    environment_pop_value(env);

    environment_push_null(env);
}

static void native_sdl_renderer_clear(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        goto leave;
    }

    SDL_RenderClear((SDL_Renderer*)native_check_pointer_value(values[0]));

leave:
    environment_pop_value(env);

    environment_push_null(env);
}

static void native_sdl_renderer_present(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        goto leave;
    }

    SDL_RenderPresent((SDL_Renderer*)native_check_pointer_value(values[0]));

leave:

    environment_pop_value(env);

    environment_push_null(env);
}

static void native_sdl_renderer_copy(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 4) {
        goto leave;
    }

    SDL_RenderCopy((SDL_Renderer*)native_check_pointer_value(values[0]),
                   (SDL_Texture*)native_check_pointer_value(values[1]),
                   (SDL_Rect*)native_check_pointer_value(values[2]),
                   (SDL_Rect*)native_check_pointer_value(values[3]));

leave:

    environment_pop_value(env);

    environment_push_null(env);
}

static void native_sdl_load_bitmap(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    SDL_Surface *bmp = NULL;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    bmp = SDL_LoadBMP(native_check_string_value(values[0]));

    if (bmp) {
        environment_push_pointer(env, (void*)bmp);
    } else {
        environment_push_null(env);
    }

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_sdl_free_surface(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        goto leave;
    }

    SDL_FreeSurface((SDL_Surface*)native_check_pointer_value(values[0]));

leave:
    environment_pop_value(env);

    environment_push_null(env);
}

static void native_sdl_create_texture_from_surface(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    SDL_Texture *texture = NULL;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);
   
    if (argc < 2) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    texture = SDL_CreateTextureFromSurface((SDL_Renderer*)native_check_pointer_value(values[0]),
                                           (SDL_Surface*)native_check_pointer_value(values[1]));

    if (texture) {
        environment_push_pointer(env, (void*)texture);
    } else {
        environment_push_null(env);
    }
    
    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_sdl_destroy_texture(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        goto leave;
    }

    SDL_DestroyTexture((SDL_Texture *)native_check_pointer_value(values[0]));

leave:
    environment_pop_value(env);
    environment_push_null(env);
}

static void native_sdl_delay(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    SDL_Window* window = NULL;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        goto leave;
    }

    SDL_Delay(native_check_int_value(values[0]));

leave:
    environment_pop_value(env);
    environment_push_null(env);
}

static void native_sdl_create_event(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;
    SDL_Event* event = NULL;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    event = (SDL_Event*)mem_alloc(sizeof(SDL_Event));

    environment_push_pointer(env, (void*)event);
}

static void native_sdl_destroy_event(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    if (argc < 1) {
        goto leave;
    }

    mem_free(native_check_pointer_value(values[0]));

leave:
    environment_push_null(env);
}

static void native_sdl_poll_event(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    if (argc < 1) {
        environment_pop_value(env);
        environment_push_null(env);
        return;
    }

    SDL_PollEvent((SDL_Event*)native_check_pointer_value(values[0]));

    environment_push_int(env, (int)((SDL_Event*)values[0]->u.pointer_value)->type);

    environment_xchg_stack(env);

    environment_pop_value(env);
}

static void native_sdl_get_ticks(environment_t env, unsigned int argc)
{
    value_t  value;
    value_t* values;

    value = list_element(list_rbegin(env->stack), value_t, link);

    values = array_base(value->u.object_value->u.array, value_t*);

    environment_pop_value(env);

    environment_push_int(env, (int)SDL_GetTicks());
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

        /* RendererFlags */
        { "SDL_RENDERER_SOFTWARE",          SDL_RENDERER_SOFTWARE },
        { "SDL_RENDERER_ACCELERATED",       SDL_RENDERER_ACCELERATED },
        { "SDL_RENDERER_PRESENTVSYNC",      SDL_RENDERER_PRESENTVSYNC },
        { "SDL_RENDERER_TARGETTEXTURE",     SDL_RENDERER_TARGETTEXTURE },
        { "SDL_WINDOW_FULLSCREEN",          SDL_WINDOW_FULLSCREEN },
        { "SDL_WINDOW_FULLSCREEN_DESKTOP",  SDL_WINDOW_FULLSCREEN_DESKTOP },
        { "SDL_WINDOW_OPENGL",              SDL_WINDOW_OPENGL },
        { "SDL_WINDOW_SHOWN",               SDL_WINDOW_SHOWN },
        { "SDL_WINDOW_HIDDEN",              SDL_WINDOW_HIDDEN },
        { "SDL_WINDOW_BORDERLESS",          SDL_WINDOW_BORDERLESS },
        { "SDL_WINDOW_RESIZABLE",           SDL_WINDOW_RESIZABLE },
        { "SDL_WINDOW_MINIMIZED",           SDL_WINDOW_MINIMIZED },
        { "SDL_WINDOW_MAXIMIZED",           SDL_WINDOW_MAXIMIZED },
        { "SDL_WINDOW_INPUT_GRABBED",       SDL_WINDOW_INPUT_GRABBED },
        { "SDL_WINDOW_INPUT_FOCUS",         SDL_WINDOW_INPUT_FOCUS },
        { "SDL_WINDOW_MOUSE_FOCUS",         SDL_WINDOW_MOUSE_FOCUS },
        { "SDL_WINDOW_FOREIGN",             SDL_WINDOW_FOREIGN },
        { "SDL_WINDOW_ALLOW_HIGHDPI",       SDL_WINDOW_ALLOW_HIGHDPI },
        { "SDL_WINDOW_MOUSE_CAPTURE",       SDL_WINDOW_MOUSE_CAPTURE },

        /* Event */
        { "SDL_FIRSTEVENT",                 SDL_FIRSTEVENT },
        { "SDL_QUIT",                       SDL_QUIT },
        { "SDL_APP_TERMINATING",            SDL_APP_TERMINATING },
        { "SDL_APP_LOWMEMORY",              SDL_APP_LOWMEMORY },
        { "SDL_APP_WILLENTERBACKGROUND",    SDL_APP_WILLENTERBACKGROUND },
        { "SDL_APP_DIDENTERBACKGROUND",     SDL_APP_DIDENTERBACKGROUND },
        { "SDL_APP_WILLENTERFOREGROUND",    SDL_APP_WILLENTERFOREGROUND },
        { "SDL_APP_DIDENTERFOREGROUND",     SDL_APP_DIDENTERFOREGROUND },
        { "SDL_WINDOWEVENT",                SDL_WINDOWEVENT },
        { "SDL_SYSWMEVENT",                 SDL_SYSWMEVENT },
        { "SDL_KEYDOWN",                    SDL_KEYDOWN },
        { "SDL_KEYUP",                      SDL_KEYUP },
        { "SDL_TEXTEDITING",                SDL_TEXTEDITING },
        { "SDL_TEXTINPUT",                  SDL_TEXTINPUT },
        { "SDL_KEYMAPCHANGED",              SDL_KEYMAPCHANGED },
        { "SDL_MOUSEMOTION",                SDL_MOUSEMOTION },
        { "SDL_MOUSEBUTTONDOWN",            SDL_MOUSEBUTTONDOWN },
        { "SDL_MOUSEBUTTONUP",              SDL_MOUSEBUTTONUP },
        { "SDL_MOUSEWHEEL",                 SDL_MOUSEWHEEL },
        { "SDL_JOYAXISMOTION",              SDL_JOYAXISMOTION },
        { "SDL_JOYBALLMOTION",              SDL_JOYBALLMOTION },
        { "SDL_JOYHATMOTION",               SDL_JOYHATMOTION },
        { "SDL_JOYBUTTONDOWN",              SDL_JOYBUTTONDOWN },
        { "SDL_JOYBUTTONUP",                SDL_JOYBUTTONUP },
        { "SDL_JOYDEVICEADDED",             SDL_JOYDEVICEADDED },
        { "SDL_JOYDEVICEREMOVED",           SDL_JOYDEVICEREMOVED },
        { "SDL_CONTROLLERAXISMOTION",       SDL_CONTROLLERAXISMOTION },
        { "SDL_CONTROLLERBUTTONDOWN",       SDL_CONTROLLERBUTTONDOWN },
        { "SDL_CONTROLLERBUTTONUP",         SDL_CONTROLLERBUTTONUP },
        { "SDL_CONTROLLERDEVICEADDED",      SDL_CONTROLLERDEVICEADDED },
        { "SDL_CONTROLLERDEVICEREMOVED",    SDL_CONTROLLERDEVICEREMOVED },
        { "SDL_CONTROLLERDEVICEREMAPPED",   SDL_CONTROLLERDEVICEREMAPPED },
        { "SDL_FINGERDOWN",                 SDL_FINGERDOWN },
        { "SDL_FINGERUP",                   SDL_FINGERUP },
        { "SDL_FINGERMOTION",               SDL_FINGERMOTION },
        { "SDL_DOLLARGESTURE",              SDL_DOLLARGESTURE },
        { "SDL_DOLLARRECORD",               SDL_DOLLARRECORD },
        { "SDL_MULTIGESTURE",               SDL_MULTIGESTURE },
        { "SDL_CLIPBOARDUPDATE",            SDL_CLIPBOARDUPDATE },
        { "SDL_DROPFILE",                   SDL_DROPFILE },
        { "SDL_AUDIODEVICEADDED",           SDL_AUDIODEVICEADDED },
        { "SDL_AUDIODEVICEREMOVED",         SDL_AUDIODEVICEREMOVED },
        { "SDL_RENDER_TARGETS_RESET",       SDL_RENDER_TARGETS_RESET },
        { "SDL_RENDER_DEVICE_RESET",        SDL_RENDER_DEVICE_RESET },
        { "SDL_USEREVENT",                  SDL_USEREVENT },
        { "SDL_LASTEVENT",                  SDL_LASTEVENT },
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
        { "init",                           native_sdl_init },
        { "quit",                           native_sdl_quit },
        { "create_window",                  native_sdl_create_window },
        { "destroy_window",                 native_sdl_destroy_window },
        { "create_renderer",                native_sdl_create_renderer },
        { "destroy_renderer",               native_sdl_destroy_renderer },
        { "renderer_clear",                 native_sdl_renderer_clear },
        { "renderer_copy",                  native_sdl_renderer_copy },
        { "renderer_present",               native_sdl_renderer_present },
        { "load_bitmap",                    native_sdl_load_bitmap },
        { "free_surface",                   native_sdl_free_surface },
        { "create_texture_from_surface",    native_sdl_create_texture_from_surface },
        { "destroy_texture",                native_sdl_destroy_texture },
        { "create_event",                   native_sdl_create_event },
        { "destroy_event",                  native_sdl_destroy_event },
        { "poll_event",                     native_sdl_poll_event },
        { "delay",                          native_sdl_delay },
        { "get_ticks",                      native_sdl_get_ticks },
    };

    for (i = 0; i < sizeof(pairs) / sizeof(struct pair_s); i++) {
        environment_push_str(env, pairs[i].name);
        environment_push_native_function(env, pairs[i].func);
        table_push_pair(sdl_table->u.object_value->u.table, env);
    }
}

#else

void import_libsdl_library(environment_t env)
{
    (void)env;
}

#endif
