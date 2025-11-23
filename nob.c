#include <stdio.h>
#include <ftw.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "src/external/nob/nob.h"

#define NAME       "cnrt"

#define SRC        "src/"
#define EXTERNAL   SRC"external/"
#define BUILD      "build/"

#define FILES      SRC"main.c"
#define RAYLIB_SRC EXTERNAL"raylib/src/"
#define NOB_LOC    EXTERNAL"nob/"
#define EXECUTABLE BUILD""NAME
#define SHELL_FILE RAYLIB_SRC"minshell.html"

// from https://stackoverflow.com/questions/5467725/how-to-delete-a-directory-and-its-contents-in-posix-c
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

// from https://stackoverflow.com/questions/5467725/how-to-delete-a-directory-and-its-contents-in-posix-c
int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

Cmd cmd = { 0 };
bool run = false;
bool web = false;

void build_raylib() {
    cmd_append(&cmd, "make");
    cmd_append(&cmd, "-C", RAYLIB_SRC);
    if (web)
        cmd_append(&cmd, "PLATFORM=PLATFORM_WEB");
}

void link_raylib() {
    cmd_append(&cmd, "-I", RAYLIB_SRC);
    cmd_append(&cmd, "-L", BUILD);
    if (web)
        cmd_append(&cmd, "-lraylib.web");
    else
        cmd_append(&cmd, "-lraylib");
}

const char *lrl_build_loc() {
    return web ? BUILD"libraylib.web.a" : BUILD"libraylib.a";
}

const char *lrl_src_loc() {
    return web ? RAYLIB_SRC"libraylib.web.a" : RAYLIB_SRC"libraylib.a";
}

void include_nob() {
    cmd_append(&cmd, "-I", NOB_LOC);
}

#ifdef __APPLE__
void macos_extras() {
    cmd_append(&cmd, "-framework", "CoreVideo");
    cmd_append(&cmd, "-framework", "IOKit");
    cmd_append(&cmd, "-framework", "Cocoa");
    cmd_append(&cmd, "-framework", "GLUT");
    cmd_append(&cmd, "-framework", "OpenGL");
}
#endif

void web_extras() {
    cmd_append(&cmd, "-o", NAME".html");
    cmd_append(&cmd, "-Os");
    cmd_append(&cmd, "-s", "USE_GLFW=3");
    cmd_append(&cmd, "-s", "ASYNCIFY");
    cmd_append(&cmd, "--shell-file", SHELL_FILE);
    cmd_append(&cmd, "-DPLATFORM_WEB");
}

void cc() {
    if (web)
        cmd_append(&cmd, "emcc");
    else
        nob_cc(&cmd);
}

void out() {
    if (web)
        cmd_append(&cmd, "-o", EXECUTABLE".html");
    else
        cmd_append(&cmd, "-o", EXECUTABLE);
}

int clean() {
    if (rmrf("build") == -1) {
        nob_log(NOB_ERROR, "Refusing to clean: %s", strerror(errno));
        return -1;
    }
    nob_log(NOB_INFO, "Removed : 'build/'");
    return 1;
}

void usage() {
    printf("./nob <Options>\n");
    printf("Options: \n");
    printf("    `r` / `run`: Runs to your code after building it.\n");
    printf("        When compined with `web`, it will run `python3 -m https.server` in the build folder.\n");
    printf("    `web`: Build for the web\n");
    printf("    `b` / `build`: Does nothing, as it always builds\n");
    printf("    `clean`: Cleans up the `build/` folder\n");
    printf("    `cleanall`: The same as `clean`, but also running `make clean` for raylib\n");
    printf("      - This is necesary when compiling to a diffrent target than the last compile\n");
    printf("    `h` / `help`: Show a help/usage message\n");

}

int run_out() {
    if (web) {
        nob_log(NOB_INFO, "Running For Web");
        cmd_append(&cmd, "mv", EXECUTABLE".html", BUILD"index.html");
        if (!cmd_run(&cmd)) return 1;

        cmd_append(&cmd, "python3");
        cmd_append(&cmd, "-m", "http.server");
        cmd_append(&cmd, "--directory", "build");
    } else {
        nob_log(NOB_INFO, "Running For Native");
        cmd_append(&cmd, "./"EXECUTABLE);
    }
    if (!cmd_run(&cmd)) return 1;
    return 0;
}

int handle_args(int argc, char **argv) {
    char *arg  = NULL;
    arg = nob_shift(argv, argc);
    if (!arg) return -1;

    if (strcmp(arg, "h") == 0 || strcmp(arg, "help") == 0) {
        usage();
        return 1;
    }
    if (strcmp(arg, "clean") == 0) {
        nob_log(NOB_INFO, "Cleaning");
        return clean();
    }
    if (strcmp(arg, "cleanall") == 0) {
        nob_log(NOB_INFO, "Cleaning All Files");
        cmd_append(&cmd, "make", "clean");
        cmd_append(&cmd, "-C", RAYLIB_SRC);
        if (!cmd_run(&cmd)) return 1;
        nob_log(NOB_INFO, "Cleaned up : ""'"RAYLIB_SRC"'");
        return clean();
    }

    for (;;) {
        if (strcmp(arg, "web") == 0) {
            web = true;
            nob_log(NOB_INFO, "Flag Set: Web Compilation");
        } else if (strcmp(arg, "r") == 0 || strcmp(arg, "run") == 0) {
            run = true;
            nob_log(NOB_INFO, "Flag Set: Run After Build");
        } else if (strcmp(arg, "b") == 0 || strcmp(arg, "build") == 0) {
        } else {
            nob_log(NOB_ERROR, "Unknown argument '%s'", arg);
            usage();
            return -1;
        }

        if (argc <= 0) break;

        arg = nob_shift(argv, argc);
    }

    return 0;
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    char *name = nob_shift(argv, argc);

    if (argc > 0) {
        int r = handle_args(argc, argv);
        if (r < 0) return 1;
        if (r > 0) return 0;
    }

    if (!mkdir_if_not_exists(BUILD)) return 1;

    if (access(lrl_build_loc(), F_OK) == -1) {
        build_raylib();
        if (!cmd_run(&cmd)) return 1;
        cmd_append(&cmd, "cp", lrl_src_loc(), BUILD);
        if (!cmd_run(&cmd)) return 1;
    }

    cc();
    nob_cc_flags(&cmd);
    cmd_append(&cmd, "--std=c99");
    cmd_append(&cmd, FILES);
    link_raylib();
#ifdef __APPLE__
    if (!web)
        macos_extras();
#endif
    if (web) web_extras();
    include_nob();
    out();
    if (!cmd_run(&cmd)) return 1;

    if (run) {
        return run_out();
    }
}
