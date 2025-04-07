#include <lauxlib.h>
#include <lualib.h>

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

static void get_table_value(lua_State *l_state, const char *name) {
    lua_pushstring(l_state, name);
    ASSERT(lua_type(l_state, -2) == LUA_TTABLE,
           "Config error: The setup() function must be called with an "
           "configuration table!");
    lua_gettable(l_state, -2);
}

static int lua_func_setup(lua_State *l_state) {
    State *state = lua_touserdata(l_state, lua_upvalueindex(1));

    for (int i = 0; i < OPTIONS_LEN; i++) {
        get_table_value(l_state, options[i].name);
        switch (options[i].type) {
        case OPTION_TYPE_BOOLEAN: {
            if (lua_isboolean(l_state, -1)) {
                options[i].value = lua_toboolean(l_state, -1);
            } else {
                WRITE_LOG("Config: The '%s' option was not specified, "
                          "defaulting to: %d",
                          options[i].name, options[i].default_value);
            }
            lua_pop(l_state, 1);
        } break;
        case OPTION_TYPE_INTEGER: {
            if (lua_isinteger(l_state, -1)) {
                options[i].value = lua_tointeger(l_state, -1);
            } else {
                WRITE_LOG("Config: The '%s' option was not specified, "
                          "defaulting to: %d",
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

    // TODO: Return a somewhat useful cano object that contains functions for
    // manipulating cano behaviour.
    // Lua code should look somewhat like this:
    //
    // local cano = setup({})
    // cano.exit(-1)
    return 0;
}

void api_init(State *state, const char *config_filename) {
    lua_State *l_state = luaL_newstate();
    if (l_state == NULL) {
        CRASH("could not initialize lua_State");
    }

    luaL_openlibs(l_state);

    lua_pushlightuserdata(l_state, state);
    lua_pushcclosure(l_state, lua_func_setup, 1);
    lua_setglobal(l_state, "setup");

    ASSERT(luaL_dofile(l_state, config_filename) == LUA_OK,
           "Failed to apply configuration: %s", lua_tostring(l_state, -1));
    lua_close(l_state);
}
