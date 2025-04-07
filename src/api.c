#include <lauxlib.h>
#include <lualib.h>
#include <stdarg.h>
#include <stdio.h>

#include "api.h"
#include "frontend.h" // IWYU pragma: keep, needed for defs.h macros (CRASH, ...)

#define OPTIONS_LEN 5
static Config_Option options[OPTIONS_LEN] = {
    {.name = "syntax", .type = OPTION_TYPE_BOOLEAN, .default_value = 1},
    {.name = "auto_indent", .type = OPTION_TYPE_BOOLEAN, .default_value = 1},
    {.name = "relative", .type = OPTION_TYPE_BOOLEAN, .default_value = 1},
    {.name = "indent", .type = OPTION_TYPE_BOOLEAN, .default_value = 0},
    {.name = "undo_size", .type = OPTION_TYPE_BOOLEAN, .default_value = 16},
};

// utility functions for interacting with the lua C api
static void get_table_value(lua_State *l_state, const char *name) {
    lua_pushstring(l_state, name);
    ASSERT(lua_type(l_state, -2) == LUA_TTABLE,
           "Config error: The setup() function must be called with an "
           "configuration table!");
    lua_gettable(l_state, -2);
}

static lua_Debug get_caller_info(lua_State *l_state) {
    lua_Debug ar;

    // the caller of the current function is on position 1
    // https://www.lua.org/manual/5.3/manual.html#lua_getstack
    if (lua_getstack(l_state, 1, &ar)) {
        // see https://www.lua.org/manual/5.3/manual.html#lua_getinfo
        // for field information
        lua_getinfo(l_state, "n", &ar);
        lua_getinfo(l_state, "S", &ar);
        lua_getinfo(l_state, "l", &ar);
        lua_getinfo(l_state, "t", &ar);
        lua_getinfo(l_state, "u", &ar);
    }
    return ar;
}

static void print_warning(lua_State *l_state, const char *fmt, ...) {
    lua_Debug db = get_caller_info(l_state);
    printf("WARNING (line %d %s): ", db.currentline, db.source);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    putchar('\n');
}

// functions exported to lua
// NOTE: All functions that are exported to lua should be prefixed with lua_.
// e.g. lua_setup
static int lua_exit(lua_State *l_state) {
    const lua_Integer code = luaL_checkinteger(l_state, 1);
    const char *msg = luaL_checkstring(l_state, 2);

    frontend_end();
    if (code < 0 || code > 255) {
        print_warning(l_state, "Consider using an integer value between 0 and "
                               "255 including for compatibility reasons.");
    }
    printf("Exiting as specified in the configuration, message: %s\n", msg);
    exit(code);
    return 0;
}

static int lua_setup(lua_State *l_state) {
    State *state = lua_touserdata(l_state, lua_upvalueindex(1));

    for (int i = 0; i < OPTIONS_LEN; i++) {
        get_table_value(l_state, options[i].name);
        switch (options[i].type) {
        case OPTION_TYPE_BOOLEAN: {
            if (lua_isboolean(l_state, -1)) {
                options[i].value = lua_toboolean(l_state, -1);
            } else {
                WRITE_LOG("Config: The '%s' option was not specified, "
                          "defaulting to: %lld",
                          options[i].name, options[i].default_value);
            }
            lua_pop(l_state, 1);
        } break;
        case OPTION_TYPE_INTEGER: {
            if (lua_isinteger(l_state, -1)) {
                options[i].value = lua_tointeger(l_state, -1);
            } else {
                WRITE_LOG("Config: The '%s' option was not specified, "
                          "defaulting to: %lld",
                          options[i].name, options[i].default_value);
            }
            lua_pop(l_state, 1);
        } break;
        }
    }

    // FIXME: Remove this temporary solution and make cano use Config_Option
    // directly. E.g. by creating an enum of indices that cano uses instead.
    // options[CONFIG_OPTION_SYNTAX].value?
    state->config.syntax = options[0].value;
    state->config.auto_indent = options[1].value;
    state->config.relative_nums = options[2].value;
    state->config.indent = options[3].value;
    state->config.undo_size = options[4].value;

    lua_newtable(l_state);
    lua_pushcfunction(l_state, lua_exit);
    lua_setfield(l_state, -2, "exit");

    return 1;
}

void api_init(State *state, const char *config_filename) {
    lua_State *l_state = luaL_newstate();
    if (l_state == NULL) {
        CRASH("could not initialize lua_State");
    }

    luaL_openlibs(l_state);

    lua_pushlightuserdata(l_state, state);
    lua_pushcclosure(l_state, lua_setup, 1);
    lua_setglobal(l_state, "setup");

    ASSERT(luaL_dofile(l_state, config_filename) == LUA_OK,
           "Failed to apply configuration: %s", lua_tostring(l_state, -1));
    lua_close(l_state);
}
