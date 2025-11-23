#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "src/external/nob/nob.h"

Cmd cmd = { 0 };

#define NAME       "tafy"

#define SRC        "src/"
#define BUILD      "build/"

#define FILES      SRC"main.c"
#define EXTERNAL   SRC"external/"
#define RAYLIB_SRC EXTERNAL"raylib/src/"
#define RAYLIB     "libraylib.a"
#define RAYLIB_LOC BUILD""RAYLIB
#define NOB_LOC    SRC"external/nob/"
#define EXECUTABLE BUILD""NAME

void build_raylib() {
    cmd_append(&cmd, "make");
    cmd_append(&cmd, "-C", RAYLIB_SRC);
}

void link_raylib() {
    cmd_append(&cmd, "-I", EXTERNAL"raygui/");
    cmd_append(&cmd, "-I", RAYLIB_SRC);
    cmd_append(&cmd, "-lraylib");
    cmd_append(&cmd, "-L", BUILD);
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

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists(BUILD)) return 1;

    if (access(RAYLIB_LOC, F_OK) == -1) {
        build_raylib();
        if (!cmd_run(&cmd)) { return 1; }
        cmd_append(&cmd, "cp", RAYLIB_SRC""RAYLIB, BUILD);
        if (!cmd_run(&cmd)) { return 1; }
    }

    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    cmd_append(&cmd, "--std=c99");
    cmd_append(&cmd, "-Wno-unused-label");
    cmd_append(&cmd, FILES);
    link_raylib();
    #ifdef __APPLE__
        macos_extras();
    #endif
    include_nob();
    cmd_append(&cmd, "-o", EXECUTABLE);
    if (!cmd_run(&cmd)) { return 1; }

    cmd_append(&cmd, "./"EXECUTABLE);
    if (!cmd_run(&cmd)) { return 1; }
}
