#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define BINARY_NAME "cano"
#define COMPILER_PATH "cc"
#define MAX_STR_LEN 512

int compile_flags_len = 6;
// pre-allocate more elements in order to append at runtime
static char *compile_flags[] = {
    "-Wall",      "-Wextra",   "-pedantic", "-Wpedantic",
    "-std=gnu11", "-Ilua/src", "-O3",       ""};

#define FILES_LEN 10
static char *source_files[FILES_LEN] = {
    SRC_FOLDER "commands.c", SRC_FOLDER "lex.c",     SRC_FOLDER "view.c",
    SRC_FOLDER "main.c",     SRC_FOLDER "cgetopt.c", SRC_FOLDER "frontend.c",
    SRC_FOLDER "keys.c",     SRC_FOLDER "buffer.c",  SRC_FOLDER "api.c",
    SRC_FOLDER "tools.c",
};

static char *out_files[FILES_LEN] = {
    BUILD_FOLDER "commands.o", BUILD_FOLDER "lex.o",
    BUILD_FOLDER "view.o",     BUILD_FOLDER "main.o",
    BUILD_FOLDER "cgetopt.o",  BUILD_FOLDER "frontend.o",
    BUILD_FOLDER "keys.o",     BUILD_FOLDER "buffer.o",
    BUILD_FOLDER "api.o",      BUILD_FOLDER "tools.o",
};

#define LIBRARIES_LEN 4
static char *libraries[LIBRARIES_LEN] = {"-lm", "-lncurses", "-L./lua/src",
                                         "-llua"};

void append_compile_flags(Cmd *cmd) {
    for (int i = 0; i < compile_flags_len; i++) {
        cmd_append(cmd, compile_flags[i]);
    }
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool install = false;
    bool debug = false;
    bool clean = false;
    char prefix[MAX_STR_LEN] = "/usr";
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--prefix", 8) == 0) {
            if (argc >= i + 1) {
                strncpy(prefix, argv[i + 1], MAX_STR_LEN);
            }
        } else if (strncmp(argv[i], "--install", 9) == 0) {
            install = true;
        } else if (strncmp(argv[i], "--clean", 9) == 0) {
            clean = true;
        } else if (strncmp(argv[i], "--help", 6) == 0) {
            printf("--prefix: Specify install prefix "
                   "(default=\"/usr\")\n"
                   "--install: Install cano to prefix directory\n"
                   "--debug: Compile cano with debug information\n"
                   "--clean: Remove build directory and uninstall if --install "
                   "is specified\n");
            return 0;
        } else if (strncmp(argv[i], "--debug", 7) == 0) {
            compile_flags[compile_flags_len++] = "-ggdb2";
            debug = true;
        }
    }

    if (!debug)
        compile_flags_len++;

    char bindir[MAX_STR_LEN];
    snprintf(bindir, MAX_STR_LEN, "%s/bin/", prefix);

    char binpath[MAX_STR_LEN];
    snprintf(binpath, MAX_STR_LEN, "%s%s", bindir, BINARY_NAME);

    char helpdir[MAX_STR_LEN];
    snprintf(helpdir, MAX_STR_LEN, "%s/share/cano/help", prefix);

    char helpdir_flag[MAX_STR_LEN];
    snprintf(helpdir_flag, MAX_STR_LEN, "-DHELP_DIR=\"%s\"", helpdir);
    compile_flags[compile_flags_len++] = helpdir_flag;

    Cmd cmd = {0};
    Cmd link_cmd = {0};
    Procs procs = {0};

    if (clean) {
        cmd_append(&cmd, "rm", "-r", BUILD_FOLDER);
        if (!cmd_run_sync_and_reset(&cmd))
            return 1;

        cmd_append(&cmd, "make", "-C", "lua", "clean");
        if (!cmd_run_sync_and_reset(&cmd))
            return 1;

        if (install) {
            cmd_append(&cmd, "rm", "-r", helpdir);
            if (!cmd_run_sync_and_reset(&cmd))
                return 1;

            if (!delete_file(binpath))
                return 1;
        }
        return 0;
    }

    if (!mkdir_if_not_exists(BUILD_FOLDER))
        return 1;

    cmd_append(&link_cmd, COMPILER_PATH);
    append_compile_flags(&link_cmd);

    for (int i = 0; i < FILES_LEN; i++) {
        nob_cmd_append(&link_cmd, out_files[i]);

        cmd_append(&cmd, COMPILER_PATH);
        append_compile_flags(&cmd);
        cmd_append(&cmd, "-c", "-o", out_files[i], source_files[i]);
        da_append(&procs, cmd_run_async_and_reset(&cmd));
    }

    if (!procs_wait_and_reset(&procs))
        return 1;

    cmd_append(&cmd, "make", "-C", "lua");
    if (!cmd_run_sync_and_reset(&cmd))
        return 1;

    cmd_append(&link_cmd, "-o", BUILD_FOLDER BINARY_NAME);
    for (int i = 0; i < LIBRARIES_LEN; i++) {
        cmd_append(&link_cmd, libraries[i]);
    }
    if (!cmd_run_sync_and_reset(&link_cmd))
        return 1;

    if (install) {
        if (!copy_directory_recursively("./docs/help/", helpdir))
            return 1;

        if (!copy_file(BUILD_FOLDER BINARY_NAME, binpath))
            return 1;
    }
    return 0;
}
