#ifndef API_H
#define API_H

#include "defs.h"

typedef enum Option_Type {
    OPTION_TYPE_BOOLEAN,
    OPTION_TYPE_INTEGER
} Option_Type;

typedef struct Config_Option {
    const char *name;
    Option_Type type;

    // may be boolean actual integer
    int value;
    int default_value;
} Config_Option;

void api_init(State *state, const char *config_filename);

#endif // !API_H
