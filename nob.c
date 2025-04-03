#include <stdio.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"

#ifdef RELEASE_MODE
#define COMPILE_FLAGS "-Wall", "-Wextra", "-pedantic", "-Isrc"
#else
#define COMPILE_FLAGS "-Wall", "-Wextra", "-pedantic", "-ggdb2"
#endif /* ifdef RELEASE_MODE */

#define BINARY_NAME "cano"

#define FILE_PATH_LEN 128
#define FILES_LEN 9

static char *source_files[FILES_LEN] = {
    SRC_FOLDER "commands.c", SRC_FOLDER "lex.c",     SRC_FOLDER "view.c",
    SRC_FOLDER "main.c",     SRC_FOLDER "cgetopt.c", SRC_FOLDER "frontend.c",
    SRC_FOLDER "keys.c",     SRC_FOLDER "buffer.c",  SRC_FOLDER "tools.c",
};

static char *out_files[FILES_LEN] = {
    BUILD_FOLDER "commands.o", BUILD_FOLDER "lex.o",
    BUILD_FOLDER "view.o",     BUILD_FOLDER "main.o",
    BUILD_FOLDER "cgetopt.o",  BUILD_FOLDER "frontend.o",
    BUILD_FOLDER "keys.o",     BUILD_FOLDER "buffer.o",
    BUILD_FOLDER "tools.o",
};

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};
    Cmd link_cmd = {0};
    Procs procs = {0};

    if (!mkdir_if_not_exists(BUILD_FOLDER))
        return 1;

    nob_cmd_append(&link_cmd, "cc", COMPILE_FLAGS);

    for (int i = 0; i < FILES_LEN; i++) {
        nob_cmd_append(&link_cmd, out_files[i]);

        cmd_append(&cmd, "cc", COMPILE_FLAGS, "-c", "-o", out_files[i],
                   source_files[i]);
        da_append(&procs, cmd_run_async_and_reset(&cmd));
    }

    if (!procs_wait_and_reset(&procs))
        return 1;

    nob_cmd_append(&link_cmd, "-o", BUILD_FOLDER BINARY_NAME, "-lm",
                   "-lncurses");
    if (!nob_cmd_run_sync_and_reset(&link_cmd))
        return 1;
    return 0;
}
