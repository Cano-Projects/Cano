#ifndef MAIN_H
#define MAIN_H

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <locale.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "buffer.h"
#include "cgetopt.h"
#include "colors.h"
#include "commands.h"
#include "defs.h"
#include "frontend.h"
#include "keys.h"
#include "lex.h"
#include "tools.h"

#define CREATE_UNDO(t, p)                                                      \
    do {                                                                       \
        Undo undo = {0};                                                       \
        undo.type = (t);                                                       \
        undo.start = (p);                                                      \
        state->cur_undo = undo;                                                \
    } while (0)

/* --------------------------- FUNCTIONS --------------------------- */

int main(int argc, char **argv);

#endif // MAIN_H
