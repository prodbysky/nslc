#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

void common_flags(Cmd* cmd) {
    cmd_append(cmd, "-Wall", "-Werror", "-g");
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    common_flags(&cmd);
    cmd_append(&cmd, "src/main.c", "-o", "nslc", "src/lexer.c");
    if (!cmd_run_sync(cmd)) return 1;

    return 0;
}
