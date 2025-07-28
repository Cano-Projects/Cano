#ifndef API_H
#define API_H

#include <lua.h>

#include "defs.h"

typedef enum Option_Type {
    OPTION_TYPE_BOOLEAN,
    OPTION_TYPE_INTEGER
} Option_Type;

typedef struct Config_Option {
    const char *name;
    Option_Type type;

    // may be a boolean or an actual integer
    lua_Integer value;
    lua_Integer default_value;
} Config_Option;

void api_init(State *state, const char *config_filename);

#endif // !API_H
